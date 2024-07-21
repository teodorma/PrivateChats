#include "DB.h"

std::mutex DB::db_mutex;
int DB::KEY_LENGTH = 512;
std::string DB::PASSWORD;
sqlite3 *DB::DataBase;
mpz_class DB::P;
mpz_class DB::Q;
mpz_class DB::N;
mpz_class DB::D;
mpz_class DB::E;


std::istringstream DB::Update(const std::string& PHONE, const std::string& IP) {
    std::lock_guard<std::mutex> guard(db_mutex);

    std::string sql = "INSERT INTO client (phone_number, name, ip) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(DataBase, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DataBase) << std::endl;
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    sqlite3_bind_text(stmt, 1, PHONE.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, PHONE.c_str(), -1, SQLITE_TRANSIENT);  // Using PHONE as the name for now
    sqlite3_bind_text(stmt, 3, IP.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert data: " << sqlite3_errmsg(DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    sqlite3_finalize(stmt);
    return std::istringstream(R"({"RESPONSE":"SUCCESS"})");
}

std::istringstream DB::Delete(const std::string& PHONE) {
    std::lock_guard<std::mutex> guard(db_mutex);

    std::string sql = "DELETE FROM client WHERE phone_number = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(DataBase, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DataBase) << std::endl;
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    sqlite3_bind_text(stmt, 1, PHONE.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to delete data: " << sqlite3_errmsg(DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    sqlite3_finalize(stmt);

    std::cout << "Success!" << std::endl;
    return std::istringstream(R"({"RESPONSE":"SUCCESS"})");
}


std::istringstream DB::Get_KEY() {
    std::lock_guard<std::mutex> guard(db_mutex);

    std::string response_str(R"({"RESPONSE":"SUCCESS", "KEY":")" + N.get_str() + R"("})");

    return std::istringstream(response_str);
}

void DB::AllData() {
    std::lock_guard<std::mutex> guard(db_mutex);

    const char* sql = "SELECT phone_number, name, ip FROM client;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(DataBase, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DataBase) << std::endl;
        return;
    }

    std::cout << "Clients in the database:" << std::endl;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char* phone_number = sqlite3_column_text(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* ip = sqlite3_column_text(stmt, 2);

        std::cout << "Phone Number: " << phone_number << ", Name: " << name << ", IP: " << ip << std::endl;
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to fetch data: " << sqlite3_errmsg(DataBase) << std::endl;
    }

    sqlite3_finalize(stmt);
}

void DB::CreateTables() {
    const char* sqlClient = R"(
        CREATE TABLE IF NOT EXISTS client (
            phone_number TEXT PRIMARY KEY NOT NULL,
            name TEXT NOT NULL,
            ip TEXT NOT NULL
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
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='client';";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(DataBase, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cout << "New instance" << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    bool tableExists = (rc == SQLITE_ROW);

    sqlite3_finalize(stmt);
    return tableExists;
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
    P = genKEY(KEY_LENGTH);
    Q = genKEY(KEY_LENGTH);
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


void DB::setPassword(){
    std::string password1, password2;
    do {
        std::cout << "Enter a strong password: ";
        std::cin >> password1;
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

DB::DB() {
    if (!isDatabaseInitiated()) {
        std::cout << "Database name: ";
        std::cin >> DB::dbName;

        int rc = sqlite3_open(dbName.c_str(), &DataBase);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(DataBase) << std::endl;
        } else {
            std::cout << "Opened database successfully" << std::endl;
        }
        CreateTables();
        setPassword();
        setKeys();
    }
    else{
    }
}
DB::~DB() = default;
