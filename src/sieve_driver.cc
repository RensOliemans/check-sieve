#include <string>
#include <fstream>
#include <sstream>

#include "sieve_driver.hh"
#include "sieve_parser.tab.hh"

sieve_driver::sieve_driver()
    : trace_scanning( false ), trace_parsing( false ), _module_map(), _command_map(), _suppress_output(false) {
    init_maps();
}

sieve_driver::sieve_driver( bool quiet )
    : trace_scanning( false ), trace_parsing( false ), _module_map(), _command_map(), _suppress_output(false) {
    init_maps();
    _suppress_output = quiet;
}

sieve_driver::~sieve_driver() {}

void sieve_driver::init_maps() {
    _command_map["keep"] = 1;
    _command_map["discard"] = 1;
    _command_map["redirect"] = 1;
    _command_map["reject"] = 1;
    _command_map["fileinto"] = 1;
    _command_map["stop"] = 1;
    _command_map["setflag"] = 1;
    _command_map["addflag"] = 1;
    _command_map["removeflag"] = 1;
    _command_map["set"] = 1;
    _command_map["vacation"] = 1;
}

int sieve_driver::parse_file( const std::string &f ) {
    file = f;
    std::ifstream fin( file );
    std::string line;
    std::string buffer;
    
    for (int i = 1; !fin.eof(); i++) {
        getline(fin, line);
        buffer += line + "\n";
    }
    
    return parse_string(buffer);
}

int sieve_driver::parse_string( const std::string &sieve ) {
    sieve_string = sieve + "\n";
    scan_begin( sieve_string );
    yy::sieve_parser parser( yyscanner, *this );
    parser.set_debug_level( trace_parsing );
    result.status = parser.parse();
    scan_end();
    return result.status;
}

void sieve_driver::error( const yy::location &l, const std::string &message, const std::string &suggestion ) {
    error( l, message );
    
    if (!_suppress_output) {
        std::cerr << std::endl << suggestion << std::endl;
    }
}

void sieve_driver::error( const yy::location &l, const std::string &message ) {
    std::istringstream f( sieve_string );
    std::string line;
    
    for (int i = 1; getline(f, line, '\n'); i++) {
        if (i == l.begin.line)
            break;
    }
 
    result.location = l;
    result.error = message;

    if (!_suppress_output) {
        std::cerr << message << std::endl;
        std::cerr << "On line " << l.begin.line << ":" << std::endl;
        std::cerr << line << std::endl;
        std::cerr << std::string(l.begin.column - 1, ' ') << std::string(l.end.column - l.begin.column, '^') << std::endl;
    }
}

void sieve_driver::error( const std::string &m ) {
    std::cerr << m << std::endl;
}

void sieve_driver::set_required_modules(std::vector<std::string> &modules) {
    for (std::vector<std::string>::const_iterator i = modules.begin(); i < modules.end(); ++i) {
        std::string str = *i;
        _module_map.insert(std::pair<std::string, bool>(str.substr(1, str.size() - 2), true));
    }
}

bool sieve_driver::supports_module(const std::string &mod) {
    return _module_map[mod];
}

bool sieve_driver::valid_command(const std::string &command) {
    return _command_map[command];
}

struct parse_result sieve_parse_file( const char *filename ) {
    sieve_driver driver;
    driver.parse_file(filename);
    return driver.result;
}

struct parse_result sieve_parse_string( const char *sieve ) {
    sieve_driver driver;
    driver.parse_string(sieve);
    return driver.result;
}