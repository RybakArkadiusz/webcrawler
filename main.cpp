#include <iostream>
#include <string>
#include <curl/curl.h>
#include <gumbo.h>
#include <vector>
#include <queue>
#include <unordered_set>
#include <cstring>
#include <thread>
#include <mutex>
#include <chrono>
#include <mysqlx/xdevapi.h>
#include "crawler.h"


int main(int argc, char* argv[]){
	if(argc<3){
		std::cerr<<"no db login credentials";
		return 1;
	}
	std::string db_login = argv[1];
	std::string db_password = argv[2];

	std::unordered_set<std::string> visited_urls;
	std::queue<std::string> queue;
	queue.push("https://www.pexels.com/");
	queue.push("https://pixabay.com/");
	queue.push("https://www.freepik.com/");
	
	queue.push("https://www.shutterstock.com/");
	queue.push("https://unsplash.com/");
	queue.push("https://www.flickr.com/");


	std::mutex queue_mutex, list_mutex;

	std::vector<std::thread> threads;
	for(int i=0; i<6; ++i){
		threads.push_back(std::thread(crawler, std::ref(queue), std::ref(visited_urls), std::ref(queue_mutex), std::ref(list_mutex), std::ref(db_login), std::ref(db_password)));

	}

	for (auto& thread : threads) {
		thread.join();
	}

	return 0;
}