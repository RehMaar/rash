#include "sig_manager.h"

#include <signal.h>

/*
static void sigint_handler() {

}
static void sigquit_handler(){

}
*/
void set_signal( void ) {
   signal(SIGINT, SIG_IGN);
   signal(SIGQUIT, SIG_IGN);
}
void set_def_signal( void ) {
   signal(SIGINT, SIG_DFL);
   signal(SIGQUIT, SIG_DFL);
}
