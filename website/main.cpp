/*
 * A website user interface for Freetron using CppCMS
 */

#include <cppcms/service.h>
#include <cppcms/mount_point.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <iostream>

#include "website.h"
#include "rpc.h"

int main(int argc, char** argv)
{
    try
    {
        cppcms::service srv(argc,argv);

        srv.applications_pool().mount(
            cppcms::applications_factory<website>(),
            cppcms::mount_point("/website")
        );

        srv.applications_pool().mount(
            cppcms::applications_factory<rpc>(),
            cppcms::mount_point("/rpc")
        );

        srv.run();
    }
    catch (std::exception const &e)
    {
        std::cerr << e.what() << std::endl;
    }
}
