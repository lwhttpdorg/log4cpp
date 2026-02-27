#!/bin/bash

# Color definitions
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

PROJ_DIR=$(realpath .)
ANALYSIS_REPORT="code-analysis.log"

dependencies=(
    "cmake"
    "clang-tidy"
)
echo -e "⏳ ${YELLOW}>>> [1/4] Checking for required dependencies...${NC}"
missing_deps=()
for dep in "${dependencies[@]}"; do
    if ! dpkg -s "$dep" &>/dev/null; then
        missing_deps+=("$dep")
    fi
done
if [ ${#missing_deps[@]} -ne 0 ]; then
    echo -e "${RED}☹️ Missing dependencies detected:${NC}"
    for dep in "${missing_deps[@]}"; do
        echo "   [x] $dep"
    done
    echo -e "${YELLOW}Please install the missing dependencies and try again.${NC}"
    echo -e "${BLUE}  sudo apt install ${missing_deps[*]}\n${NC}"
    exit 1
fi
echo -e "✔️ ${GREEN}Dependencies check passed.\n${NC}"

echo -e "⏳ ${YELLOW}>>> [2/4] Cleaning up old files...${NC}"
rm -rf ${ANALYSIS_REPORT} build
echo -e "✔️ ${GREEN}Cleanup complete.\n${NC}"

echo -e "⏳ ${YELLOW}>>> [3/4] Configuring CMake...${NC}"
cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build \
    -DBUILD_LOG4CPP_DEMO=OFF -DENABLE_LOG4CPP_UNIT_TEST=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Check if CMake configuration succeeded
if [ $? -ne 0 ]; then
    echo -e "❌ ${RED}ERROR: CMake configuration failed!${NC}"
    exit 1
fi
echo -e "✔️ ${GREEN}CMake configuration successful. compile_commands.json generated.\n${NC}"

echo -e "⏳ ${YELLOW}>>> [4/4] Running clang-tidy and saving output to ${ANALYSIS_REPORT}...${NC}"
echo -e "${BLUE}Note: This may take a while depending on the project size...${NC}"

start_time=$(date +%s)

# Execute clang-tidy and capture both stdout and stderr
run-clang-tidy -p build -header-filter="^${PROJ_DIR}/.*" > ${ANALYSIS_REPORT} 2>&1

# Calculate duration
end_time=$(date +%s)
duration=$((end_time - start_time))

# Check for counts in the log file
# Using variable names consistently now
errors=$(grep -c "error:" ${ANALYSIS_REPORT} || true)
warnings=$(grep -c "warning:" ${ANALYSIS_REPORT} || true)
notes=$(grep -c "note:" ${ANALYSIS_REPORT} || true)

echo -e "${GREEN}--------------------------------------------------${NC}"
echo -e "✔️ Code analysis complete! Time elapsed: ${duration} seconds."
echo -e "Summary:"
echo -e "  ${RED}error:   $errors${NC}"
echo -e "  ${YELLOW}warning: $warnings${NC}"
echo -e "  ${BLUE}note:    $notes${NC}"
echo -e "${GREEN}--------------------------------------------------\n${NC}"

# If any count is non-zero, print in RED. Otherwise, print in GREEN.
if [ "$errors" -gt 0 ] || [ "$warnings" -gt 0 ] || [ "$notes" -gt 0 ]; then
    echo -e "☹️ ${RED}STATUS: code analysis NOT PASSED.${NC}"
    echo -e "${YELLOW}Check '${ANALYSIS_REPORT}' for details.${NC}"
else
    echo -e "✔️ ${GREEN}STATUS: code analysis PASSED.${NC}"
fi

