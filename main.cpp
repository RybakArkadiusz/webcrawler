#include <iostream>
#include <string>
#include <curl/curl.h>
#include <gumbo.h>
#include <vector>
#include <stack>
#include <unordered_set>
#include <cstring>
#include <thread>
#include <mutex>
#include <chrono>


//todo: error handling, waiting untill stack is empty and all threads are done instead of closing threads whenever stack is empty even if it can be filled up in a second

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){//tak to musi wyglądać żeby curl to przyjął czyli chyba bufor, contents to to co curl otrzymuje,size to size nmemb to to ile jest elementów w buforze a userp to wskaźnik na to w czym chcemy mieć zapisane dane np później do std::string
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}


std::vector<std::string> find_links(GumboNode* node) {
    std::vector<std::string> links;
    if (node->type != GUMBO_NODE_ELEMENT) {
        return links;
    }
    GumboAttribute* href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
    		if(std::strncmp(href->value, "http://", 7)!=0 || std::strncmp(href->value, "https://", 8)!=0){
        	links.push_back(href->value);
        }
    }
    GumboVector* children = &node->v.element.children;
      for (unsigned int i = 0; i < children->length; ++i) {
      GumboNode* child_node = static_cast<GumboNode*>(children->data[i]);
      std::vector<std::string> child_links = find_links(static_cast<GumboNode*>(children->data[i]));
      for (const auto& child_link : child_links) {
          if (child_link.compare(0, 7, "http://") == 0 || child_link.compare(0, 8, "https://") == 0) {
              links.push_back(child_link);
          }
      }
    }
    return links;
}



int make_absolute_url(const std::string& base_url, std::vector<std::string>& links){
	int count = 0;
	for(auto& s:links){
		if(s[0]=='/'){
			s= base_url+s;
			++count;
		}
	}
	return count;
}


void crawler(std::stack<std::string>& url_stack, std::unordered_set<std::string>& visited_urls, std::mutex& stack_mutex, std::mutex& list_mutex){
	std::string url, readBuffer;
	GumboOutput* g_output;
	while(1){
		{
			std::lock_guard<std::mutex> lock(stack_mutex);
			if(url_stack.empty()){
				std::cout<<std::this_thread::get_id()<<" empty stack\n";
				break;
			}
			url = url_stack.top();
			url_stack.pop();
		}

		{
			std::lock_guard<std::mutex> lock(list_mutex);
			if(visited_urls.find(url) != visited_urls.end()) {
      	continue;
    	}
    	visited_urls.insert(url);
		}

		CURL *curl = curl_easy_init();
		readBuffer.clear();

		if(curl) {
			CURLcode res;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
	  }
		const char *readBufferChar = readBuffer.c_str();
		std::cout<<std::this_thread::get_id()<<":   "<<url<<"\n";

		g_output = gumbo_parse(readBufferChar);
		std::vector<std::string> extracted_links = find_links(g_output->root);

		{
			std::lock_guard<std::mutex> lock(stack_mutex);
			for(const auto& link: extracted_links){
				url_stack.push(link);
			}
		}
	}
}



int main(void){
	std::unordered_set<std::string> visited_urls;
	std::stack<std::string> stack;
	stack.push("https://pl.wikipedia.org/wiki/Robot_internetowy");
	stack.push("https://www.google.com/");
	stack.push("https://www.goal.com/en");
	stack.push("https://www.livescore.com/en/");
	stack.push("https://open.spotify.com/");
	stack.push("https://www.foxnews.com/");


	std::mutex stack_mutex, list_mutex;

	std::vector<std::thread> threads;
	for(int i=0; i<6; ++i){
		threads.push_back(std::thread(crawler, std::ref(stack), std::ref(visited_urls), std::ref(stack_mutex), std::ref(list_mutex)));

	}

	for (auto& thread : threads) {
		thread.join();
	}


	return 0;
}