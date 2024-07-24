#!/bin/bash

# Clone and build sqlcipher
git clone https://github.com/sqlcipher/sqlcipher.git
cd sqlcipher

# Install OpenSSL
install_openssl() {
    case "$1" in
        "Ubuntu"|"Debian")
            echo "Detected $1. Installing openssl using apt."
            sudo apt update
            sudo apt install -y openssl libssl-dev
            ;;
        "Fedora")
            echo "Detected Fedora. Installing openssl using dnf."
            sudo dnf install -y openssl openssl-devel
            ;;
        "CentOS"|"RedHatEnterpriseServer"|"Rocky"|"AlmaLinux")
            echo "Detected $1. Installing openssl using yum."
            sudo yum install -y openssl openssl-devel
            ;;
        "openSUSE"|"SUSE")
            echo "Detected $1. Installing openssl using zypper."
            sudo zypper install -y openssl libopenssl-devel
            ;;
        "Arch"|"Manjaro")
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
        "Ubuntu"|"Debian")
            echo "Detected $1. Installing jsoncpp-devel using apt."
            sudo apt update
            sudo apt install -y jsoncpp-devel
            ;;
        "Fedora")
            echo "Detected Fedora. Installing jsoncpp-devel using dnf."
            sudo dnf install -y jsoncpp-devel
            ;;
        "CentOS"|"RedHatEnterpriseServer"|"Rocky"|"AlmaLinux")
            echo "Detected $1. Installing jsoncpp-devel using yum."
            sudo yum install -y jsoncpp-devel
            ;;
        "openSUSE"|"SUSE")
            echo "Detected $1. Installing jsoncpp-devel using zypper."
            sudo zypper install -y jsoncpp-devel
            ;;
        "Arch"|"Manjaro")
            echo "Detected $1. Installing jsoncpp-devel using pacman."
            sudo pacman -Syu jsoncpp
            ;;
        *)
            echo "Unsupported OS: $1. Please install jsoncpp-devel manually."
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

# Build sqlcipher
./configure --enable-tempstore=yes CFLAGS="-DSQLITE_HAS_CODEC" LDFLAGS="-lcrypto"
make
sudo make install

cd ... || exit

# Install jsoncpp-devel
install_jsoncpp "$OS_NAME"

# Build the project
mkdir build
cd build

cmake ..
make

./Server

cd ... || exit
