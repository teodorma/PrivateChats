#!/bin/bash

# Clone and build sqlcipher
git clone https://github.com/sqlcipher/sqlcipher.git
cd sqlcipher

# Install OpenSSL
install_openssl() {
    case "$1" in
        "ubuntu"|"debian")
            echo "Detected $1. Installing openssl using apt."
            sudo apt update
            sudo apt install -y openssl libssl-dev
            ;;
        "fedora")
            echo "Detected Fedora. Installing openssl using dnf."
            sudo dnf install -y openssl openssl-devel
            ;;
        "centos"|"rhel"|"rocky"|"almalinux")
            echo "Detected $1. Installing openssl using yum."
            sudo yum install -y openssl openssl-devel
            ;;
        "opensuse"|"suse")
            echo "Detected $1. Installing openssl using zypper."
            sudo zypper install -y openssl libopenssl-devel
            ;;
        "arch"|"manjaro")
            echo "Detected $1. Installing openssl using pacman."
            sudo pacman -Syu openssl
            ;;
        *)
            echo "Unsupported OS: $1. Please install openssl manually."
            exit 1
            ;;
    esac
}

# Function to install jsoncpp-devel using the appropriate package manager
install_jsoncpp() {
    case "$1" in
        "ubuntu"|"debian")
            echo "Detected $1. Installing jsoncpp-devel using apt."
            sudo apt update
            sudo apt install -y libjsoncpp-dev
            ;;
        "fedora")
            echo "Detected Fedora. Installing jsoncpp-devel using dnf."
            sudo dnf install -y jsoncpp-devel
            ;;
        "centos"|"rhel"|"rocky"|"almalinux")
            echo "Detected $1. Installing jsoncpp-devel using yum."
            sudo yum install -y jsoncpp-devel
            ;;
        "opensuse"|"suse")
            echo "Detected $1. Installing jsoncpp-devel using zypper."
            sudo zypper install -y jsoncpp-devel
            ;;
        "arch"|"manjaro")
            echo "Detected $1. Installing jsoncpp-devel using pacman."
            sudo pacman -Syu jsoncpp
            ;;
        *)
            echo "Unsupported OS: $1. Please install jsoncpp-devel manually."
            exit 1
            ;;
    esac
}

# Function to install sqlite3 using the appropriate package manager
install_sqlite3() {
    case "$1" in
        "ubuntu"|"debian")
            echo "Detected $1. Installing sqlite3 using apt."
            sudo apt update
            sudo apt install -y sqlite3 libsqlite3-dev
            ;;
        "fedora")
            echo "Detected Fedora. Installing sqlite3 using dnf."
            sudo dnf install -y sqlite sqlite-devel
            ;;
        "centos"|"rhel"|"rocky"|"almalinux")
            echo "Detected $1. Installing sqlite3 using yum."
            sudo yum install -y sqlite sqlite-devel
            ;;
        "opensuse"|"suse")
            echo "Detected $1. Installing sqlite3 using zypper."
            sudo zypper install -y sqlite3 sqlite3-devel
            ;;
        "arch"|"manjaro")
            echo "Detected $1. Installing sqlite3 using pacman."
            sudo pacman -Syu sqlite
            ;;
        *)
            echo "Unsupported OS: $1. Please install sqlite3 manually."
            exit 1
            ;;
    esac
}

# Function to install gmp-devel using the appropriate package manager
install_gmp() {
    case "$1" in
        "ubuntu"|"debian")
            echo "Detected $1. Installing gmp using apt."
            sudo apt update
            sudo apt install -y libgmp-dev
            ;;
        "fedora")
            echo "Detected Fedora. Installing gmp-devel using dnf."
            sudo dnf install -y gmp-devel
            ;;
        "centos"|"rhel"|"rocky"|"almalinux")
            echo "Detected $1. Installing gmp-devel using yum."
            sudo yum install -y gmp-devel
            ;;
        "opensuse"|"suse")
            echo "Detected $1. Installing gmp-devel using zypper."
            sudo zypper install -y gmp-devel
            ;;
        "arch"|"manjaro")
            echo "Detected $1. Installing gmp using pacman."
            sudo pacman -Syu gmp
            ;;
        *)
            echo "Unsupported OS: $1. Please install gmp-devel manually."
            exit 1
            ;;
    esac
}

# Function to update cmake using the appropriate package manager
update_cmake() {
    case "$1" in
        "ubuntu"|"debian")
            echo "Detected $1. Updating cmake using apt."
            sudo apt update
            sudo apt install -y cmake
            ;;
        "fedora")
            echo "Detected Fedora. Updating cmake using dnf."
            sudo dnf install -y cmake
            ;;
        "centos"|"rhel"|"rocky"|"almalinux")
            echo "Detected $1. Updating cmake using yum."
            sudo yum install -y cmake
            ;;
        "opensuse"|"suse")
            echo "Detected $1. Updating cmake using zypper."
            sudo zypper install -y cmake
            ;;
        "arch"|"manjaro")
            echo "Detected $1. Updating cmake using pacman."
            sudo pacman -Syu cmake
            ;;
        *)
            echo "Unsupported OS: $1. Please update cmake manually."
            exit 1
            ;;
    esac
}

# Detect the OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS_NAME=$ID
else
    echo "Cannot determine the operating system."
    exit 1
fi

# Install OpenSSL
install_openssl "$OS_NAME"

# Install sqlite3
install_sqlite3 "$OS_NAME"

# Update cmake
update_cmake "$OS_NAME"

# Install gmp-devel
install_gmp "$OS_NAME"

# Build sqlcipher
./configure --enable-tempstore=yes CFLAGS="-DSQLITE_HAS_CODEC" LDFLAGS="-lcrypto"
make
sudo make install

cd - || exit

# Install jsoncpp-devel
install_jsoncpp "$OS_NAME"

# Build the project
mkdir build
cd build

cmake ..
make

./Server

cd - || exit
