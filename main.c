#include <stdio.h>
#include <string.h>
#include <stdlib.h> // exit() and atoi()
#include <stdbool.h> // true/false

// setuid()
#include <sys/types.h>
#include <sys/stat.h> // chmod
#include <sys/mount.h> // mout fs
#include <unistd.h> // cwd

// err //
#include <errno.h>

// helpers //
#include "utils.h"

// globals //
static const char g_Permissions[] = "0777";

// typedefs //
typedef void (*cbAtExit)(void);

static void dummy_at_exit(void);

/// registers a callback when exit() is called if needed to handle stuff
/// \brief register_exit_callback
/// \param ex - callback to the exit
///
static void register_exit_callback(cbAtExit ex);

/// trims N elements from the end of a string by a delimiter
/// \brief trim_end
/// \param str - returned new modified string
/// \param delim - pattern
/// \param count - elements to cut off
///
static void trim_end(char* str, const char delim, int count);

/// used for dynamic trimming of unknown string size
/// count delimiters in a string
/// \brief delimiter_count
/// \param str
/// \param delim
/// \return
static int delimiter_count(const char* str, char delim);

/// mount filesystem with specific options
/// \brief mount_filesystem
/// \param src - source to mount ex. "Nothin"
/// \param tgt - target ex /var/tmp
/// \param fstype - type tmpfs
/// \param flags - user defined flags, see man mount(2)
/// \param mode - octal mode like 0777
/// \param uid - user id 65534 like for nobody
/// \return
///
static int mount_filesystem(const char* src, const char* tgt, const char* fstype,
                            unsigned long flags, const char* mode, const char* uid);

struct strlist_t
{
    char** list;
    int size;
};

int main(int argc, char* argv[])
{
    (void) argc; // unused
    // avoid compile warinings for unised
    register_exit_callback(dummy_at_exit);
    bool matched_result = false;

    // extra size
     char pwd[512]={0};
     getcwd(pwd, sizeof(pwd)/sizeof(pwd[0]));

     printf("Current working dir is: %s\n", pwd);

    // Check is already mounted ramfs on the current directory.
    // The beter way is manual parsing.
    // in order to avoid that process we have to write dirwalker
    // to list all mounted fses but for now will use this one
     FILE* mntstatus = fopen("/etc/mtab", "r");
     // make goro happy
     if (mntstatus == NULL) {
         fprintf(stderr, "Error opening \"/etc/mtab\"!\n");
         exit(3);
     }

    // assume no more than 512 chars per line
    char buff[512]={0};

    struct strlist_t s_list = {0, 0};

    while (fgets(buff, 512, mntstatus) != NULL) {
        const char* match = strchr2(buff, '/', 1);
        // trim 3 whitespaces from the end
        int delimiters = delimiter_count(match, ' ');

        // trimming an original const string is not good
        // idea, better to use my split later
        // to just access second element
        trim_end((char*) match, ' ', delimiters); // when parsing mtab

        if (strcmp(match, pwd)==0) {
            printf("Match: [%s]\n", match);
            matched_result = true;
            break; // no need to search more
        }
    }
    fclose(mntstatus);

    // our cwd is not mounted
    if (!matched_result) {
        // empty, ok we can mount here
        // just efective-uid is not enough for mount
        // i need real-uid
        setuid(0);

        // now correctly using the custom function istead of system
        int res = mount_filesystem("ramfs", pwd, "ramfs", 0, "rw", 0);
        if (res == 0) {
            printf("Mount ok\n");
            // convert to octals
            int octal_perms = strtol(g_Permissions, 0, 8);
            if (chmod(pwd, octal_perms) < 0) {
                fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                        argv[0], // program
                        pwd,    // current dir
                        g_Permissions, // with permissions
                        errno,          // Ermac :)
                        strerror(errno));
                exit(1);
            }

        } else {
            printf("Mount failed!\n");
            return 1;
        }
    } else {
        printf("Dude you are ok!\n");
    }

    return 0;
}


void trim_end(char *str, const char delim, int count)
{
    char* begin = str;
    char* end = &str[strlen(str)-1];
    int i=0;
    while ((i < count) && ( begin != end)) {
        if (*end == delim) {
            i++;
        }
        *end-- = 0;
    }
}

static int delimiter_count(const char* str, char delim)
{
    int i = 0;
    while (*str != '\0') {
        if (*str == delim) {
            i++;
        }
        str++;
    }

    return i;
}

void dummy_at_exit(void)
{

}


void register_exit_callback(cbAtExit ex)
{
    if (ex) {
        atexit(ex);
    }
}

int mount_filesystem(const char *src, const char *tgt, const char *fstype, unsigned long flags,
                     const char *mode, const char *uid)
{
    char mode_uid[256]={0};
    if((mode != NULL) && (uid != NULL)) {
        sprintf(mode_uid, "mode=%s,uid=%s", mode, uid);
    }

    int result = mount(src, tgt, fstype, flags, mode_uid);
    // handle result outside
    return result;
}


/// yeah, eat this BASH!
