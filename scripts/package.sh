NETID="sww35"
DIRNAME="${NETID}_EC3"

# Create directory structure
mkdir -p $DIRNAME
mkdir -p $DIRNAME/src
mkdir -p $DIRNAME/include
mkdir -p $DIRNAME/test
mkdir -p $DIRNAME/examples

# Copy source files
cp src/*.cpp $DIRNAME/src/
cp include/*.hpp $DIRNAME/include/
cp CMakeLists.txt $DIRNAME/
cp README.md $DIRNAME/
cp test/test_cases.md $DIRNAME/test/
cp examples/addresses.txt $DIRNAME/examples/

# Create ZIP file
zip -r "${NETID}_EC3.zip" $DIRNAME

echo "Created ${NETID}_EC3.zip ready for submission"
