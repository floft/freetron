/*
 * Freetron - an open-source software scantron implementation
 *
 * Todo:
 *   - Statistics (e.g. most missed, least missed, ...?) as text and/or images
 *   - Generalize for any style of form, make forms/type*.xml and autodetect
 *   - When a box is missing, calculate supposed position
 *   - Pick largest image on the page of a PDF
 *   - Rotate based on a few boxes in line, then find all boxes
 *   - Auto-adjusting HEIGHT_ERROR and MIN_BLACK
 */

#include <list>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <IL/il.h>
#include <cppcms/service.h>
#include <cppcms/mount_point.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>

#include "options.h"
#include "processor.h"
#include "website/database.h"
#include "website/rpc.h"
#include "website/website.h"

// This must be global since we're using extern in options.h. This is used all
// over the place to enable outputting debug images.
bool DEBUG = false;

enum class Args
{
    Unknown,
    Help,
    Threads,
    Debug,
    ID,
    DB,
    Daemon,
    SiteConfig,
    Max
};

void help()
{
    std::cerr << "Usage" << std::endl
              << "  freetron [options] --daemon website/" << std::endl
              << "  freetron [options] -i KeyID form.pdf" << std::endl
              << std::endl
              << "Options" << std::endl
              << "  -i, --id           ID of form to use as the key (if not daemon)" << std::endl
              << "  -d, --debug        output debug images" << std::endl
              << "  -t, --threads #    max number of threads to create" << std::endl
              << "  --daemon website/  run the website, don't exit till Ctrl+C" << std::endl
              << "  --config conf.js   alternate config (no path)" << std::endl
              << "  --db sqlite.db     alternate database (no path)" << std::endl
              << "  --max 250          max upload filesize in megabytes" << std::endl;
}

void invalid()
{
    std::cerr << "Invalid argument (see \"-h\" for usage)" << std::endl;
    std::exit(1);
}

int main(int argc, char* argv[])
{
    // Argument parsing
    std::string path;
    std::string filename;
    std::string siteconfig = "config.js";
    std::string database = "sqlite.db";
    bool daemon = false;
    int threads = 0; // 0 == number of cores
    long long key = DefaultID;
    long long maxFilesize = 250*1024*1024;

    std::map<std::string, Args> options = {{
        { "-h",        Args::Help },
        { "--help",    Args::Help },
        { "-t",        Args::Threads },
        { "--threads", Args::Threads },
        { "-d",        Args::Debug },
        { "--debug",   Args::Debug },

        // Daemon specific
        { "-i",        Args::ID },
        { "--id",      Args::ID },

        // Website specific
        { "--daemon",  Args::Daemon },
        { "--config",  Args::SiteConfig },
        { "--db",      Args::DB },
        { "--max",     Args::Max }
    }};

    for (int i = 1; i < argc; ++i)
    {
        switch (options[argv[i]])
        {
            case Args::Help:
                help();
                return 1;
            case Args::ID:
                ++i;

                if (i == argc)
                    invalid();

                try
                {
                    key = std::stoll(argv[i]);
                }
                catch (const std::invalid_argument&)
                {
                    std::cerr << "Error: invalid key ID" << std::endl;
                    return 1;
                }
                catch (const std::out_of_range&)
                {
                    std::cerr << "Error: key ID too long" << std::endl;
                    return 1;
                }
                break;
            case Args::Threads:
                ++i;

                if (i == argc)
                    invalid();

                try
                {
                    threads = std::stoi(argv[i]);
                }
                catch (const std::invalid_argument&)
                {
                    std::cerr << "Error: invalid number of threads" << std::endl;
                    return 1;
                }
                catch (const std::out_of_range&)
                {
                    std::cerr << "Error: max threads too long" << std::endl;
                    return 1;
                }
                break;
            case Args::Debug:
                DEBUG = true;
                break;
            case Args::Daemon:
                ++i;
                daemon = true;

                if (i == argc)
                    invalid();

                path = argv[i];
                break;
            case Args::SiteConfig:
                ++i;

                if (i == argc)
                    invalid();

                siteconfig = argv[i];
                break;
            case Args::DB:
                ++i;

                if (i == argc)
                    invalid();

                database = argv[i];
                break;
            case Args::Max:
                ++i;

                if (i == argc)
                    invalid();

                try
                {
                    maxFilesize = std::stoll(argv[i]);
                }
                catch (const std::invalid_argument&)
                {
                    std::cerr << "Error: invalid max filesize" << std::endl;
                    return 1;
                }
                catch (const std::out_of_range&)
                {
                    std::cerr << "Error: max filesize too large" << std::endl;
                    return 1;
                }
                break;
            default:
                if (!filename.empty())
                    invalid();

                filename = argv[i];
                break;
        }
    }


    if (key == DefaultID && !daemon)
    {
        std::cerr << "Error: key ID cannot be the default ID" << std::endl;
        return 1;
    }

    ilInit();

    if (!daemon)
    {
        Database db;
        Processor p(threads, false, db);

        // Process a single form and exit
        p.add(0, key, filename);
        p.wait();
        std::cout << p.print(0);
    }
    else
    {
        try
        {
            // Go to the website directory
            if (chdir(path.c_str()) != 0)
                throw std::runtime_error("couldn't set directory");

            // Load config
            std::ifstream configFile(siteconfig);

            if (!configFile.is_open())
                throw std::runtime_error("couldn't open config file");

            int configError = 0;
            cppcms::json::value config;

            if (!config.load(configFile, true, &configError))
                throw std::runtime_error("config error on line " + std::to_string(configError));

            // Check that uploads/ and files/ subdirectories exist
            struct stat info;

            if (stat("uploads", &info) != 0 || !(info.st_mode&S_IFDIR))
                throw std::runtime_error("couldn't find uploads/ subdirectory");

            if (stat("files", &info) != 0 || !(info.st_mode&S_IFDIR))
                throw std::runtime_error("couldn't find files/ subdirectory");

            // Init database
            Database db(database);

            // Init application
            Processor p(threads, true, db);

            // Init website
            cppcms::service srv(config);

            srv.applications_pool().mount(
                cppcms::applications_factory<website,WebsiteData>(
                    WebsiteData(db, p, maxFilesize)),
                cppcms::mount_point("/website")
            );

            srv.applications_pool().mount(
                cppcms::applications_factory<rpc,Database&,Processor&>(db, p),
                cppcms::mount_point("/rpc")
            );

            // TODO: Look for any leftover forms in the upload directory

            // Run website and block
            srv.run();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    return 0;
}
