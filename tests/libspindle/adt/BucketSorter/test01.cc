//
//
//

#include "spindle/ArrayBucketSorter.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
    ArrayBucketSorter bs(10,10);
    bs.printDebug();
    bs.insert(2,0);
    bs.insert(2,2);
    bs.insert(2,4);
    bs.insert(2,6);
    bs.insert(2,8);
    bs.printDebug();
    bs.dump();
    bs.reset();
    bs.setInsertBack();
    bs.printDebug();
    bs.insert(2,0);
    bs.insert(2,2);
    bs.insert(2,4);
    bs.insert(2,6);
    bs.insert(2,8);
    bs.printDebug();
    bs.dump();
}
