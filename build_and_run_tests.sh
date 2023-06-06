echo Build ...
CURRENT_DIR=$(pwd)
SCRIPT_DIR=$(dirname "$0")
BUILD_DIR=$SCRIPT_DIR/build

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake -DBUILD_CSDF_TESTS=True -DBUILD_CSDF_PTHREAD_SUPPORT=True -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
cd $CURRENT_DIR

echo
echo Run tests ...
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=$BUILD_DIR/tests/valgrind_out.txt \
         ./$BUILD_DIR/tests/tests
