#include "DB.h"

std::mutex DB::db_mutex;
int DB::KEY_LENGTH = 512;
std::string DB::PASSWORD;
sqlite3 *DB::DataBase = nullptr;
mpz_class DB::P;
mpz_class DB::Q;
mpz_class DB::N;
mpz_class DB::D;
mpz_class DB::E;


void DB::createTables() {
    const char* sqlClient = R"(
        CREATE TABLE IF NOT EXISTS client (
            phone_number TEXT PRIMARY KEY NOT NULL,
            name TEXT NOT NULL,
            key TEXT NOT NULL
        );
    )";

    const char* sqlAdmin = R"(
        CREATE TABLE IF NOT EXISTS admin (
            id INTEGER PRIMARY KEY CHECK (id = 0),
            password TEXT NOT NULL,
            P TEXT NOT NULL,
            Q TEXT NOT NULL,
            N TEXT NOT NULL,
            D TEXT NOT NULL,
            E TEXT NOT NULL
        );
    )";

    char* errorMessage = nullptr;

    // Execute SQL for client table
    int rc = sqlite3_exec(DataBase, sqlClient, nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error in client table creation: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "Client table created successfully" << std::endl;
    }

    // Execute SQL for admin table
    rc = sqlite3_exec(DataBase, sqlAdmin, nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error in admin table creation: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "Admin table created successfully" << std::endl;
    }
}



bool DB::isDatabaseInitiated() {
    // Check if the database file exists
    std::ifstream file(DATABASE_NAME);
    if (!file) {
        return false;
    }
    else{
        return true;
    }
//
//    // Open the database
//    int rc = sqlite3_open(DATABASE_NAME, &DataBase);
//    if (rc != SQLITE_OK) {
//        std::cerr << "Cannot open database: " << sqlite3_errmsg(DataBase) << std::endl;
//        return false;
//    }
//
//    // Debug: Check if the password is correctly set
//    if (PASSWORD.empty()) {
//        std::cerr << "The password hash is empty. Ensure setPassword() is called before this function." << std::endl;
//        sqlite3_close(DataBase);
//        return false;
//    }
//
//    // Set the key for SQLCipher
//    std::string keyPragma = "PRAGMA key = '" + PASSWORD + "';";
//    char* errorMessage = nullptr;
//    rc = sqlite3_exec(DataBase, keyPragma.c_str(), nullptr, nullptr, &errorMessage);
//    if (rc != SQLITE_OK) {
//        std::cerr << "Failed to set key: " << errorMessage << std::endl;
//        sqlite3_free(errorMessage);
//        sqlite3_close(DataBase);
//        return false;
//    }
//
//    // Prepare the SQL statement to check for the 'client' table
//    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='client';";
//    sqlite3_stmt* stmt;
//    rc = sqlite3_prepare_v2(DataBase, sql, -1, &stmt, nullptr);
//
//    if (rc != SQLITE_OK) {
//        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DataBase) << std::endl;
//        sqlite3_close(DataBase);
//        return false;
//    }
//
//    rc = sqlite3_step(stmt);
//    bool tableExists = (rc == SQLITE_ROW);
//
//    sqlite3_finalize(stmt);
//    sqlite3_close(DataBase);
//
//    return tableExists;
}



void DB::setKeys() {
    std::string len;
    while (true) {
        std::cout << "Enter the length of the RSA key (default 512/max 2048): ";
        std::getline(std::cin, len);
        if (len.empty()) {
            std::cout << "Key length set to 512." << std::endl;
            KEY_LENGTH = 512;
            break;
        }
        try {
            int len_int = std::stoi(len);
            if (len_int >= 512 && len_int <= 2048) {
                std::cout << "Key length set to " << len_int << std::endl;
                KEY_LENGTH = len_int;
                break;
            } else {
                std::cout << "The length of the key must be higher than 512 and lower than 2048." << std::endl;
            }
        } catch (const std::invalid_argument&) {
            std::cout << "Invalid input. Please enter a valid number." << std::endl;
        } catch (const std::out_of_range&) {
            std::cout << "The number is too large. Please enter a smaller number." << std::endl;
        }
    }

    // Generate RSA keys
    P = genKEY(KEY_LENGTH/2);
    Q = genKEY(KEY_LENGTH/2);
    N = P * Q;
    E = 65537;
    D = calculatePrivateExponent(P, Q, E);

    std::cout << "P: " << P.get_str()
              << "\nQ: " << Q.get_str()
              << "\nN: " << N.get_str()
              << "\nD: " << D.get_str() << std::endl;

    // Store keys into SQLite table
    const std::string sql = R"(
        INSERT INTO admin (id, password, P, Q, N, D, E)
        VALUES (0, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(DataBase, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DataBase) << std::endl;
        return;
    }

    // Bind password and RSA keys to the SQL statement
    sqlite3_bind_text(stmt, 1, PASSWORD.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, P.get_str().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, Q.get_str().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, N.get_str().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, D.get_str().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, E.get_str().c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert data: " << sqlite3_errmsg(DataBase) << std::endl;
    } else {
        std::cout << "Keys successfully inserted into the database." << std::endl;
    }

    sqlite3_finalize(stmt);
}



bool DB::passCheck(const std::string& password){
    if(password.length() < 8) return false;

    const char num[] = "1234567890";
    const char caps[] = "QWERTYUIOPASDFGHJKLZXCVBNM";
    const char spec[] = "~!@#$%^&*(){}|<>?:";

    bool special;
    bool capt;
    bool number;

    for(auto chr : password){
        if(strchr(num, chr)) number = true;
        if(strchr(caps, chr)) capt = true;
        if(strchr(spec, chr)) special = true;
        if(number && special && capt) return true;
    }
    return false;
}


void DB::createDatabase() {
    int rc = sqlite3_open(DATABASE_NAME, &DataBase);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(DataBase) << std::endl;
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    std::string keyCommand = "PRAGMA key = '" + PASSWORD + "';";
    char* errorMessage = nullptr;
    rc = sqlite3_exec(DataBase, keyCommand.c_str(), nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to set encryption key: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    }
}

void DB::setPassword(){
    std::string password1, password2;
    do {
        std::cout << "Enter a strong password: ";
        std::cin >> password1;
        if(!passCheck(password1)){
            std::cout << "Password is too weak\n";
            continue;
        }
        std::cout << "Retype the password: ";
        std::cin >> password2;

        if (password1 != password2) {
            std::cout << "Passwords do not match. Please try again." << std::endl;
        }
    } while (password1 != password2);

    PASSWORD = sha256(password1);
    password1.clear();
    password2.clear();

    // Clear any leftover newline characters from the input buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}


bool DB::setRSAParameters() {
    int rc;
    // Prepare SQL query
    sqlite3_stmt* stmt;
    const char* sql = "SELECT P, Q, D, N, E FROM admin LIMIT 1;";
    rc = sqlite3_prepare_v2(DataBase, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DataBase) << std::endl;
        sqlite3_close(DataBase);
        return false;
    }

    // Execute query and retrieve the values
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        P.set_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)), 10);
        Q.set_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)), 10);
        D.set_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)), 10);
        N.set_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)), 10);
        E.set_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)), 10);
    } else {
        std::cerr << "No data found: " << sqlite3_errmsg(DataBase) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(DataBase);
        return false;
    }
    std::cout << "P: " << P.get_str()
              << "\nQ: " << Q.get_str()
              << "\nN: " << N.get_str()
              << "\nD: " << D.get_str() << std::endl;

    // Clean up
    sqlite3_finalize(stmt);
    return true;
}

void DB::openDatabase() {
    int rc = sqlite3_open(DATABASE_NAME, &DataBase);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(DataBase) << std::endl;
        return;
    }

    std::cout << "Opened database successfully" << std::endl;

    std::string keyCommand = "PRAGMA key = '" + PASSWORD + "';";
    char* errorMessage = nullptr;
    rc = sqlite3_exec(DataBase, keyCommand.c_str(), nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to set encryption key: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(DataBase);
        return;
    }

    // Verify if the database is opened successfully with the password
    const char* sql = "PRAGMA user_version;";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(DataBase, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare verification statement: " << sqlite3_errmsg(DataBase) << std::endl;
        sqlite3_close(DataBase);
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
        std::cerr << "Database verification failed: " << sqlite3_errmsg(DataBase) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(DataBase);
        return;
    } else {
        std::cout << "Database opened successfully with the provided password." << std::endl;
    }

    sqlite3_finalize(stmt);
}



DB::DB() {
    if (!isDatabaseInitiated()) {
        setPassword();
        createDatabase();
        createTables();
        setKeys();
    } else {
        std::cout << "Enter password for database: ";
        std::string pass;
        std::cin >> pass;
        PASSWORD = sha256(pass);
        pass.clear();
        openDatabase();
        setRSAParameters();
    }

}




DB::~DB() {
    if (DataBase) {
        sqlite3_close(DataBase);
    }
}