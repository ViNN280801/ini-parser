#include <iostream>

#include "iniparser.h"

int main()
{
    ini_error_details_t err;
    if (ini_good("ini_example.ini").error != INI_SUCCESS)
    {
        std::cerr << "Error: " << ini_error_details_to_string(ini_good("ini_example.ini")) << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "Success when checking INI file." << std::endl;
    }

    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        std::cerr << "Failed to create INI context." << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "Success when creating INI context." << std::endl;
    }

    err = ini_load(ctx, "ini_example.ini");
    if (err.error != INI_SUCCESS)
    {
        std::cerr << "Error: " << ini_error_details_to_string(err) << std::endl;
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
        {
            std::cerr << "Error: " << ini_error_details_to_string(err) << std::endl;
        }
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "Success when loading INI file." << std::endl;
    }
    err = ini_print(ctx);
    if (err.error != INI_SUCCESS)
    {
        std::cerr << "Error: " << ini_error_details_to_string(err) << std::endl;
    }
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
    {
        std::cerr << "Error: " << ini_error_details_to_string(err) << std::endl;
    }

    return EXIT_SUCCESS;
}
