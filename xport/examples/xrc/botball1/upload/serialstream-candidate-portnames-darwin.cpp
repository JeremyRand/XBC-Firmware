#include <sys/types.h>
#include <dirent.h>

vector<string>
SerialStream::candidatePortnames()
{
  //FILE *debugout=fopen("/tmp/serial-cand.txt","w");
  vector<string> ret;
  string directory="/dev";


  DIR *d= opendir(directory.c_str());
    
  if (!d) {
    //if (debugout) fprintf(debugout, "couldn't open /dev\n");
    //fflush(debugout);
    //fclose(debugout);
    return ret;
  }

  //fprintf(debugout, "about to read directory\n");
  //fflush(debugout);
  struct dirent *ent;
  while ((ent= readdir(d))) {
    //fprintf(debugout, "found >%s<\n", ent->d_name);
    //fflush(debugout);
    if (!strncmp(ent->d_name, "tty.", 4)) {
      // Skip tty.modem as accessing it really wedges the system under 10.1.2
      if (strcmp(ent->d_name, "tty.modem")) {
        ret.push_back(directory + "/" + ent->d_name);
      }
    }
  }
  closedir(d);
  //fclose(debugout);

  return ret;
}
