#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <ini.h>
#include <platform/unix.h>

typedef struct {
  char *section;
  char **exec;
} desktop_t;

// A function to handle .desketop lines
static int desktop_handler(void* user, const char* section, const char* name, const char* value)
{
  desktop_t* pdesktop = (desktop_t*) user;
  if (!strcmp(pdesktop->section, section) && !strcmp(name, EXEC)) {
    *pdesktop->exec = malloc(sizeof(char)*(strlen(value) + 1));
    strcpy(*pdesktop->exec, value);
  }
}

// A function to determine if a file exists in the filesystem
bool file_exists(char *path)
{
  if(access(path,R_OK) == 0) {
    return true;
  }
  else {
    return false;  
  }
}

// A function to remove field codes from .desktop file Exec line
void strip_field_codes(char *cmd)
{
  int start = 0;
  for (int i = 0; i < strlen(cmd); i++) {
    if (cmd[i] == '%' && i > 0 && cmd[i - 1] == ' ') {
      start = i;
    }
    else if (start && i > start + 2 && cmd[i] != ' ') {
      strcpy(cmd + start, cmd + i);
      start = 0;
    }
  }
  if (start) {
    cmd[start - 1] ='\0'; 
  }
}

// A function to determine if the command is the path to a .desktop file
// and retrieve the Exec command if so
int parse_desktop_file(char *command, char **exec)
{
  char *cmd = malloc(sizeof(char)*(strlen(command) + 1));
  strcpy(cmd, command);
  char *desktop_file = strtok(cmd, DELIMITER_ACTION);
  if (strlen(desktop_file) > EXT_DESKTOP_LENGTH) {
    char *extension = cmd + (strlen(desktop_file) - 8);
    if (!strcmp(extension, EXT_DESKTOP)) {
      desktop_t desktop;
      char *action = strtok(NULL,DELIMITER_ACTION);
      if (action == NULL) {
        desktop.section = malloc(sizeof(char)*(DESKTOP_SECTION_HEADER_LENGTH + 1));
        strcpy(desktop.section,DESKTOP_SECTION_HEADER);
      }
      else {
        desktop.section = malloc(sizeof(char)*(DESKTOP_SECTION_HEADER_ACTION_LENGTH
        + strlen(action) + 1));
        sprintf(desktop.section, DESKTOP_SECTION_HEADER_ACTION, action);
      }
      desktop.exec = exec;
      char *file = desktop_file;
      int error = ini_parse(file, desktop_handler, &desktop);
      free(desktop.section);
      if (error < 0) {
        printf("Error: Desktop file \"%s\" not found\n", cmd);
        free(cmd);
        return DESKTOP_ERROR;
      }
      else if (*exec == NULL) {
        printf("No Exec line found in desktop file \"%s\"\n", cmd);
        free(cmd);
        return DESKTOP_ERROR;
      }
      strip_field_codes(exec);
      free(cmd);
      return DESKTOP_SUCCESS;
    }
  }
  free(cmd);
  return DESKTOP_NOT_FOUND;
}