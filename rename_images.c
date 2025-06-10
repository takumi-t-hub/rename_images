#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>

// List of image file extensions to process
const char *EXT_LIST[] = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".heic", ".dng", ".tif", ".mov", ".mp4"};
const int EXT_COUNT = 10;

// Function to check if a file has an image extension
int is_image_file(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot)
        return 0;

    for (int i = 0; i < EXT_COUNT; i++)
    {
        if (strcasecmp(dot, EXT_LIST[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

// Function to get file extension
const char *get_file_extension(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    return dot ? dot : "";
}

int cmpstr(const void *a, const void *b)
{
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcmp(fa, fb);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s prefix digits\n", argv[0]);
        return 1;
    }

    const char *prefix = argv[1];
    int digits = atoi(argv[2]);
    if (digits <= 0 || digits > 20)
    {
        printf("Invalid digits: %s\n", argv[2]);
        return 1;
    }
    DIR *dir;
    struct dirent *entry;
    char **image_files = NULL;
    int image_count = 0;
    int array_size = 10;

    // Open current directory
    dir = opendir(".");
    if (dir == NULL)
    {
        perror("Unable to open directory");
        return 1;
    }

    // Allocate initial array for image filenames
    image_files = malloc(array_size * sizeof(char *));
    if (!image_files)
    {
        perror("Memory allocation failed");
        closedir(dir);
        return 1;
    }

    // First pass: collect all image files
    while ((entry = readdir(dir)) != NULL)
    {
        if (is_image_file(entry->d_name))
        {
            // Resize array if needed
            if (image_count >= array_size)
            {
                array_size *= 2;
                char **temp = realloc(image_files, array_size * sizeof(char *));
                if (!temp)
                {
                    perror("Memory reallocation failed");
                    for (int i = 0; i < image_count; i++)
                    {
                        free(image_files[i]);
                    }
                    free(image_files);
                    closedir(dir);
                    return 1;
                }
                image_files = temp;
            }

            // Store filename
            image_files[image_count] = strdup(entry->d_name);
            if (!image_files[image_count])
            {
                perror("String duplication failed");
                for (int i = 0; i < image_count; i++)
                {
                    free(image_files[i]);
                }
                free(image_files);
                closedir(dir);
                return 1;
            }
            image_count++;
        }
    }
    closedir(dir);

    // Sort image_files lexicographically
    qsort(image_files, image_count, sizeof(char *), cmpstr);

    // Second pass: rename files
    for (int i = 0; i < image_count; i++)
    {
        const char *ext = get_file_extension(image_files[i]);
        char ext_upper[32];
        int j = 0;
        for (; ext[j] && j < (int)sizeof(ext_upper) - 1; j++)
        {
            ext_upper[j] = (char)toupper((unsigned char)ext[j]);
        }
        ext_upper[j] = '\0';

        char new_name[256];
        snprintf(new_name, sizeof(new_name), "%s%0*d%s", prefix, digits, i + 1, ext_upper);

        if (rename(image_files[i], new_name) != 0)
        {
            printf("Error renaming %s to %s: %s\n", image_files[i], new_name, strerror(errno));
        }
        else
        {
            printf("Renamed %s to %s\n", image_files[i], new_name);
        }
        free(image_files[i]);
    }

    free(image_files);
    return 0;
}
