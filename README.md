# webcrawler
Description

This C++ project is a multithreaded web crawler designed to scrape images from various websites and store them into a MySQL database. It leverages the libcurl library for making HTTP requests and Gumbo for parsing HTML content. The crawler offers efficient image extraction capabilities, making it ideal for web scraping tasks requiring parallel processing and database integration.

Usage

To use the compiled program, you'll need to provide your MySQL database credentials (username and password) as command-line arguments. Here's an example command:
./main username password
Replace username and password with your MySQL credentials.

Dependencies

libcurl: https://curl.se/libcurl/
Gumbo HTML Parser: https://github.com/google/gumbo-parser
MySQL Connector/C++: https://dev.mysql.com/doc/connector-cpp/8.3/en/
