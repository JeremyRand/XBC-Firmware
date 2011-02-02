#include <qapplication.h>
#include "martemainform.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MarteMainForm w;
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
