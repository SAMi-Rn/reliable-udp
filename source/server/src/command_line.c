#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glob.h>
#include <stdbool.h>
#include <errno.h>
#include <inttypes.h>
#include "command_line.h"

int parse_arguments(int argc, char *argv[], char **server_addr,
                char **client_addr, char **server_port_str,
                char **client_port_str, uint8_t *window_size,
                struct fsm_error *err)
{
    int opt;
    bool C_flag, c_flag, S_flag, s_flag, w_flag;

    opterr = 0;
    C_flag = 0;
    c_flag = 0;
    S_flag = 0;
    s_flag = 0;
    w_flag = 0;

    while ((opt = getopt(argc, argv, "C:c:S:s:w:h")) != -1)
    {
        switch (opt)
        {
            case 'C':
            {
                if (C_flag)
                {
                    char message[40];

                    snprintf(message, sizeof(message), "option '-C' can only be passed in once.");
                    usage(argv[0]);
                    SET_ERROR(err, message);

                    return -1;
                }

                C_flag++;
                *client_addr = optarg;
                break;
            }
            case 'c':
            {
                if (c_flag)
                {
                    char message[40];

                    snprintf(message, sizeof(message), "option '-c' can only be passed in once.");
                    usage(argv[0]);
                    SET_ERROR(err, message);

                    return -1;
                }

                c_flag++;
                *client_port_str = optarg;
                break;
            }
            case 'S':
            {
                if (S_flag)
                {
                    char message[40];

                    snprintf(message, sizeof(message), "option '-S' can only be passed in once.");
                    usage(argv[0]);
                    SET_ERROR(err, message);

                    return -1;
                }

                S_flag++;
                *server_addr = optarg;
                break;
            }
            case 's':
            {
                if (s_flag)
                {
                    char message[40];

                    snprintf(message, sizeof(message), "option '-s' can only be passed in once.");
                    usage(argv[0]);
                    SET_ERROR(err, message);

                    return -1;
                }

                s_flag++;
                *server_port_str = optarg;
                break;
            }
            case 'w':
            {
                if (w_flag)
                {
                    char message[40];

                    snprintf(message, sizeof(message), "option '-w' can only be passed in once.");
                    usage(argv[0]);
                    SET_ERROR(err, message);

                    return -1;
                }

                w_flag++;
                char *temp;
                temp = optarg;

                if (convert_to_int(argv[0], temp, window_size, err) == -1)
                {
                    return -1;
                }
                break;
            }
            case 'h':
            {
                usage(argv[0]);
                SET_ERROR(err, "user called for help");

                return -1;
            }
            case '?':
            {
                char message[24];

                snprintf(message, sizeof(message), "Unknown option '-%c'.", optopt);
                usage(argv[0]);
                SET_ERROR(err, message);

                return -1;
            }
            default:
            {
                usage(argv[0]);
            }
        }
    }
//    if (optind == argc)
//    {
//        SET_ERROR(err, "Need to pass at least one file to send.");
//        char message[40];
//
//        snprintf(message, sizeof(message), "Need to pass at least one file to send.");
//        usage(argv[0]);
//        return -1;
//    }

//    for (int i = optind; i < argc; i++)
//    {
//        get_files(argv[i], glob_result, err);
//    }

    return 0;
}

void usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s [-i] <value> [-p] <value> [-h] <value>\n", program_name);
    fputs("Options:\n", stderr);
    fputs("  -h                     Display this help message\n", stderr);
    fputs("  -c <value>             Option 'c' (required) with value, Sets the IP client_addr\n", stderr);
    fputs("  -s <value>             Option 's' (required) with value, Sets the IP server_addr\n", stderr);
    fputs("  -p <value>             Option 'p' (required) with value, Sets the Port\n", stderr);
}

int                 handle_arguments(const char *binary_name, const char *server_addr,
                                     const char *client_addr, const char *server_port_str,
                                     const char *client_port_str, in_port_t *server_port,
                                     in_port_t *client_port, struct fsm_error *err)
{
    if(server_addr == NULL)
    {
        SET_ERROR(err, "The server_addr is required.");
        usage(binary_name);

        return -1;
    }

    if(client_addr == NULL)
    {
        SET_ERROR(err, "The client_addr is required.");
        usage(binary_name);

        return -1;
    }

    if(server_port_str == NULL)
    {
        SET_ERROR(err, "The server_port is required.");
        usage(binary_name);

        return -1;
    }

    if(client_port_str == NULL)
    {
        SET_ERROR(err, "The server_port is required.");
        usage(binary_name);

        return -1;
    }

    if (parse_in_port_t(binary_name, server_port_str, server_port, err) == -1)
    {
        return -1;
    }

    if (parse_in_port_t(binary_name, client_port_str, client_port, err) == -1)
    {
        return -1;
    }

    return 0;
}

int parse_in_port_t(const char *binary_name, const char *str, in_port_t *port, struct fsm_error *err)
{
    char            *endptr;
    uintmax_t       parsed_value;

    errno           = 0;
    parsed_value    = strtoumax(str, &endptr, 10);

    if(errno != 0)
    {
        SET_ERROR(err, strerror(errno));

        return -1;
    }

    // Check if there are any non-numeric characters in the input string
    if(*endptr != '\0')
    {
        SET_ERROR(err, "Invalid characters in input.");
        usage(binary_name);

        return -1;
    }

    // Check if the parsed value is within the valid range for in_port_t
    if(parsed_value > UINT16_MAX)
    {
        SET_ERROR(err, "in_port_t value out of range.");
        usage(binary_name);

        return -1;
    }

    *port = (in_port_t)parsed_value;
    return 0;
}

int convert_to_int(const char *binary_name, char *string, uint8_t *value, struct fsm_error *err)
{
    char            *endptr;
    uintmax_t       parsed_value;

    errno = 0;
    parsed_value = strtoumax(string, &endptr, 10);

    if (errno != 0)
    {
        SET_ERROR(err, strerror(errno));

        return -1;
    }

    if(*endptr != '\0')
    {
        SET_ERROR(err, "Invalid characters in input.");
        usage(binary_name);

        return -1;
    }

    if (parsed_value > 100)
    {
        char error_message[25];
        snprintf(error_message, sizeof(error_message), "%s value out of range.", string);
        SET_ERROR(err, error_message);
        usage(binary_name);

        return -1;
    }

    *value = (uint8_t) parsed_value;

    return 0;
}
