#include <dlfcn.h>
#include <stdio.h>

#include <cstring>

#include "driver_log.h"
#include "file_path.h"

void test_func() {

}

std::string GetDriverBinPath() {
  std::string selfPath;
  Dl_info di;
  dladdr((void*)GetDriverBinPath, &di);
  return di.dli_fname;
}

//std::string GetPathByFileName(std::string targetFilename)
//{
//  FILE *fp = fopen("/proc/self/maps", "r");
//  if (NULL == fp) {
//    return "";
//  }
//  const size_t BUFFER_SIZE = 256;
//  char buffer[BUFFER_SIZE] = "";
//  char path[BUFFER_SIZE] = "";
//
//  while (fgets(buffer, BUFFER_SIZE, fp)) {
//    if (sscanf(buffer, "%*llx-%*llx %*s %*s %*s %*s %s", path) == 1) {
//      char *bname = basename(path);
//      if (strcasecmp(bname, targetFilename.c_str()) == 0) {
//        fclose(fp);
//        return path;
//      }
//    }
//  }
//  fclose(fp);
//  return "";
//}