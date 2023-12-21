#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Warning: if you change the value of MAX_ID_LEN, you might need to change the
// length of the format_specifier string.

#define MAX_ID_LEN 32   // The maximum length of an instance ID
#define MAX_NAME_LEN 64 // The maximum length of an instance name

/**
 * Writes the name of the instance that was terminated to a file.
 *
 * @param instance_name The name of the instance that was terminated.
 */
void write_terminated_instances_file(const char *instance_name)
{
    const char *file_name = "terminated_instances.txt";
    FILE *fd;
    fd = fopen(file_name, "a");
    if (NULL != fd)
    {
        fprintf(fd, "%s\n", instance_name);
        fclose(fd);
    }
}

/**
 * Terminates an AWS instance.
 *
 * @param instance_name The name of the instance to terminate. Max length is 64.
 */
void terminate_aws_instance(const char *instance_name)
{
    FILE *fd;
    // Maximum length of the instance ID (it's actually shorter, but we're being
    // safe for future-proofing)
    char instance_id[MAX_ID_LEN + 1];

    // Length of the AWS command plus the maximum length of the instance name
    char get_id_command[187 + (MAX_NAME_LEN + 1)];

    // Length of the command to terminate the instance plus the max length of
    // the instance ID
    char command[58 + (MAX_ID_LEN + 1)];

    // (% + s + null terminator) + character length of MAX_ID_LEN
    char format_specifier[3 + 2];

    snprintf(format_specifier, sizeof(format_specifier), "%%%ds", MAX_ID_LEN); // "%ds"

    snprintf(get_id_command, sizeof(get_id_command), "aws ec2 describe-instances --filters \"Name=tag:Name,Values=%s\" \"Name=instance-state-name,Values=running\" --query \"Reservations[*].Instances[*].InstanceId\" --output text > instance_id.txt", instance_name);

    system(get_id_command);

    fd = fopen("instance_id.txt", "r");
    if (NULL != fd) // If the file exists
    {
        fseek(fd, 0, SEEK_END);
        if (ftell(fd) != 0) // If the file is not empty
        {
            fseek(fd, 0, SEEK_SET);

            fscanf(fd, format_specifier, instance_id);

            fclose(fd);

            snprintf(command, sizeof(command), "aws ec2 terminate-instances --instance-ids %s > /dev/null", instance_id);

            system(command);

            remove("instance_id.txt");

            write_terminated_instances_file(instance_name);
        }
    }
}

/**
 * Checks if an instance was already terminated.
 *
 * @param instance_name The name of the instance to check.
 * @return 1 if the instance was already terminated, 0 otherwise.
 */
int was_instance_already_terminated(const char *instance_name)
{
    const char *file_name = "terminated_instances.txt";
    FILE *fd;
    char line[MAX_NAME_LEN + 1];
    int was_terminated = 0;

    fd = fopen(file_name, "r");
    if (NULL != fd)
    {
        while (fgets(line, sizeof(line), fd))
        {
            if (strcmp(line, instance_name) == 0)
            {
                was_terminated = 1;
                break;
            }
        }
        fclose(fd);
    }

    return was_terminated;
}