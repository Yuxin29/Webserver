#!/bin/bash

# Get script directory to ensure paths work from anywhere
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Banner
echo -e "${BOLD}${BLUE}"
echo "╔══════════════════════════════════════════════════════╗"
echo "║                                                      ║"
echo "║         WEBSERV CGI COMPREHENSIVE TEST SUITE        ║"
echo "║                                                      ║"
echo "╚══════════════════════════════════════════════════════╝"
echo -e "${NC}\n"

# Check if webserv is running
echo -e "${YELLOW}Checking if webserver is running on port 8081...${NC}"
if ! nc -z localhost 8081 2>/dev/null; then
    echo -e "${RED}ERROR: Webserver is not running on port 8081${NC}"
    echo -e "${YELLOW}Please start webserv first: ./webserv configuration/webserv.conf${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Webserver is running${NC}\n"

sleep 1

# Function to run a test suite
run_test_suite() {
    local script=$1
    local description=$2
    
    echo -e "${BOLD}${BLUE}"
    echo "════════════════════════════════════════════════════════"
    echo "  $description"
    echo "════════════════════════════════════════════════════════"
    echo -e "${NC}\n"
    
    if [ -f "$script" ]; then
        bash "$script"
        echo -e "\n${GREEN}✓ $description completed${NC}\n"
    else
        echo -e "${RED}✗ Test script not found: $script${NC}\n"
    fi
    
    sleep 2
}

# Main menu
echo -e "${BOLD}Select test suite to run:${NC}"
echo -e "  ${GREEN}1${NC} - GET Request Tests"
echo -e "  ${GREEN}2${NC} - POST Request Tests"
echo -e "  ${GREEN}3${NC} - Chunked Transfer-Encoding Tests"
echo -e "  ${GREEN}4${NC} - Run ALL CGI Tests"
echo -e "  ${GREEN}5${NC} - Run ALL Tests (including chunked files)"
echo -e "  ${GREEN}q${NC} - Quit"
echo -ne "\n${YELLOW}Enter your choice [1-5/q]: ${NC}"

read -r choice

case $choice in
    1)
        run_test_suite "$SCRIPT_DIR/test_cgi_get.sh" "GET REQUEST TESTS"
        ;;
    2)
        run_test_suite "$SCRIPT_DIR/test_cgi_post.sh" "POST REQUEST TESTS"
        ;;
    3)
        run_test_suite "$SCRIPT_DIR/test_cgi_chunked.sh" "CHUNKED TRANSFER-ENCODING TESTS"
        ;;
    4)
        run_test_suite "$SCRIPT_DIR/test_cgi_get.sh" "GET REQUEST TESTS"
        run_test_suite "$SCRIPT_DIR/test_cgi_post.sh" "POST REQUEST TESTS"
        run_test_suite "$SCRIPT_DIR/test_cgi_chunked.sh" "CHUNKED TRANSFER-ENCODING TESTS"
        ;;
    5)
        run_test_suite "$SCRIPT_DIR/test_cgi_get.sh" "GET REQUEST TESTS"
        run_test_suite "$SCRIPT_DIR/test_cgi_post.sh" "POST REQUEST TESTS"
        run_test_suite "$SCRIPT_DIR/test_cgi_chunked.sh" "CHUNKED CGI TESTS"
        run_test_suite "$SCRIPT_DIR/test_chunked_files.sh" "CHUNKED FILE UPLOAD TESTS"
        ;;
    q|Q)
        echo -e "${YELLOW}Exiting...${NC}"
        exit 0
        ;;
    *)
        echo -e "${RED}Invalid choice${NC}"
        exit 1
        ;;
esac

echo -e "\n${BOLD}${GREEN}"
echo "╔══════════════════════════════════════════════════════╗"
echo "║                                                      ║"
echo "║              ALL TESTS COMPLETED!                    ║"
echo "║                                                      ║"
echo "╚══════════════════════════════════════════════════════╝"
echo -e "${NC}\n"
