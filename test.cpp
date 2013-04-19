
// mapnik
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/util/conversions.hpp>

// geos
#include <geos_c.h>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/process_cpu_clocks.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

using namespace boost::chrono;


// stl
#include <exception>
#include <iostream>
#include <fstream>
#include <vector>

static unsigned test_num = 1;
static bool dry_run = false;
static std::set<int> test_set;

typedef process_cpu_clock clock_type;
typedef clock_type::duration dur;

template <typename T>
void benchmark(T & test_runner, std::string const& name)
{
    try {
        bool should_run_test = true;
        if (!test_set.empty())
        {
            should_run_test = test_set.find(test_num) != test_set.end();
        }
        if (should_run_test || dry_run)
        {
            if (!test_runner.validate())
            {
                std::clog << "test did not validate: " << name << "\n";
                //throw std::runtime_error(std::string("test did not validate: ") + name);
            }
            if (dry_run)
            {
                std::clog << test_num << ") " << (test_runner.threads_ ? "threaded -> ": "")
                    << name << "\n";
            }
            else
            {
                process_cpu_clock::time_point start;
                dur elapsed;
                if (test_runner.threads_ > 0)
                {
                    boost::thread_group tg;
                    for (unsigned i=0;i<test_runner.threads_;++i)
                    {
                        tg.create_thread(test_runner);
                        //tg.create_thread(boost::bind(&T::operator(),&test_runner));
                    }
                    start = process_cpu_clock::now();
                    tg.join_all();
                    elapsed = process_cpu_clock::now() - start;
                }
                else
                {
                    start = process_cpu_clock::now();
                    test_runner();
                    elapsed = process_cpu_clock::now() - start;
                }
                std::clog << test_num << ") " << (test_runner.threads_ ? "threaded -> ": "")
                    << name << ": "
                    << boost::chrono::duration_cast<milliseconds>(elapsed) << "\n";
            }
        }
    }
    catch (std::exception const& ex)
    {
        std::clog << "test runner did not complete: " << ex.what() << "\n";
    }
    test_num++;
}

struct test_mapnik
{
    std::vector<std::string> & wkt_;
    unsigned iter_;
    unsigned threads_;
    test_mapnik(std::vector<std::string> & wkt,
                unsigned iterations,
                unsigned threads=0) :
      wkt_(wkt),
      iter_(iterations),
      threads_(threads)
      {}

    bool validate()
    {
        return true;
    }

    void operator()()
    {
        for (unsigned i=0;i<iter_;++i) {
            mapnik::wkt_parser parse_wkt;
            BOOST_FOREACH(std::string const& wkt, wkt_)
            {
                boost::ptr_vector<mapnik::geometry_type> paths;
                //if (!mapnik::from_wkt(wkt, paths)) // slow, grammar per parse
                if (!parse_wkt.parse(wkt, paths)) // fast, grammar re-use
                {
                    throw std::runtime_error("Failed to parse WKT");
                }
            }
        }
    }
};


void geos_notice(const char* format, ...)
{
#ifdef MAPNIK_LOG
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 512, format, args);
    va_end(args);

    MAPNIK_LOG_WARN(geos) << "geos_datasource: " << buffer;
#endif
}

void geos_error(const char* format, ...)
{
#ifdef MAPNIK_LOG
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 512, format, args);
    va_end(args);

    MAPNIK_LOG_ERROR(geos) << "geos_datasource: " << buffer;
#endif
}


struct test_geos
{
    std::vector<std::string> & wkt_;
    unsigned iter_;
    unsigned threads_;
    test_geos(std::vector<std::string> & wkt,
                unsigned iterations,
                unsigned threads=0) :
      wkt_(wkt),
      iter_(iterations),
      threads_(threads)
      {}

    bool validate()
    {
        return true;
    }

    void operator()()
    {
         GEOSWKTReader * reader = GEOSWKTReader_create();
         for (unsigned i=0;i<iter_;++i) {
             BOOST_FOREACH(std::string const& wkt, wkt_)
             {
                 GEOSGeometry* geometry = GEOSWKTReader_read(reader, wkt.c_str());
                 if (!geometry)
                 {
                     throw std::runtime_error("GEOS Plugin: invalid <wkt> geometry specified");
                 }
                 GEOSGeom_destroy(geometry);
             }
         }
         GEOSWKTReader_destroy(reader);
    }
};


int main( int argc, char** argv)
{
    if (argc > 0) {
        for (int i=0;i<argc;++i) {
            std::string opt(argv[i]);
            if (opt == "-d" || opt == "--dry-run") {
                dry_run = true;
            } else if (opt[0] != '-') {
                int arg;
                if (mapnik::util::string2int(opt,arg)) {
                    test_set.insert(arg);
                }
            }
        }
    }
    try {
        std::string filename("./cases/wkt.csv");
        std::ifstream stream(filename.c_str(),std::ios_base::in | std::ios_base::binary);
        if (!stream.is_open())
            throw std::runtime_error("could not open: '" + filename + "'");

        std::vector<std::string> wkt_list;
        std::string csv_line;
        while(std::getline(stream,csv_line,'\n'))
        {
            if (csv_line.empty() || csv_line[0] == '#') continue;
            std::string dequoted = csv_line.substr(1,csv_line.size()-2);
            wkt_list.push_back(dequoted);
        }
        stream.close();
        
        unsigned iterations = 10000;
        unsigned threads = 10;

        // mapnik
        {
            test_mapnik runner(wkt_list,iterations,threads);
            benchmark(runner, "mapnik");
        }

        // geos
        {
            initGEOS(geos_notice, geos_error);
            test_geos runner(wkt_list,iterations,threads);
            benchmark(runner, "geos");
        }
    }
    catch (std::exception const& ex)
    {
        std::cerr << ex.what() << "\n";
    }
    return 0;
}
