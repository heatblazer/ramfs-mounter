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

// globals //
static const char g_Permissions[] = "0777";

// typedefs //
typedef void* (*cbAtExit)(void);

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


int main(int argc, char* argv[])
{
    (void) argc; // unused

    bool matched_result = false;

    // extra size
     char pwd[512]={0};
     getcwd(pwd, sizeof(pwd)/sizeof(pwd[0]));

     printf("Current working dir is: %s\n", pwd);

    // Check is already mounted ramfs on the current directory.
    // The beter way is manual parsing.
    // in order to avoid that process we have to write dirwalker
    // to list all mounted fses but for now will use this one
    FILE* mntstatus = popen("mount", "r");

    // assume no more than 512 chars per line
    char buff[512]={0};

    while (fgets(buff, 512, mntstatus) != NULL) {
        char* match = strchr(buff, '/');
        // trim 3 whitespaces from the end
        trim_end(match, ' ', 3);
        if (strcmp(match, pwd)==0) {
            printf("Match: [%s]\n", match);
            matched_result = true;
            break; // no need to search more
        }
    }

    // our cwd is not mounted
    if (!matched_result) {
        // empty, ok we can mount here
        // just efective-uid is not enough for mount
        // i need real-uid
        setuid(0);

        //int res = mount_filesystem(pwd, "ramfs", "ramfs", 0, "rw", 0);
        int res = system("mount -t ramfs ramfs `pwd`");
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


void register_exit_callback(cbAtExit ex)
{
    atexit(ex);
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
