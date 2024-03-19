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
#include <fstream>


std::string _FILENAME = "img.txt";
//todo: waiting untill queue is empty and all threads are done instead of closing threads whenever queue is empty even if it can be filled up in a second

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

std::vector<std::string> find_image_links(GumboNode* node){
	std::vector<std::string> links;
	if (node->type != GUMBO_NODE_ELEMENT) {
    return links;
  }

  if (node->v.element.tag == GUMBO_TAG_IMG &&
    	gumbo_get_attribute(&node->v.element.attributes, "src")) {
    const char* src = gumbo_get_attribute(&node->v.element.attributes, "src")->value;
    links.push_back(src);
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    GumboNode* child_node = static_cast<GumboNode*>(children->data[i]);
    std::vector<std::string> child_images = find_image_links(child_node);
    links.insert(links.end(), child_images.begin(), child_images.end());
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


void crawler(std::queue<std::string>& url_queue, std::unordered_set<std::string>& visited_urls, std::mutex& queue_mutex, std::mutex& list_mutex, std::mutex& file_mutex){
	std::string url, readBuffer;
	GumboOutput* g_output;
	bool flag = 1;
	std::vector<std::string> extracted_image_links;

	while(1){
		if(flag){
			std::lock_guard<std::mutex> lock(queue_mutex);
			if(url_queue.empty()){
				//std::cerr<<std::this_thread::get_id()<<" empty queue\n";
				continue;
			}
			url = url_queue.front();
			url_queue.pop();
		}

		

		{
			std::lock_guard<std::mutex> lock(list_mutex);
			if(visited_urls.find(url) != visited_urls.end()) {
				flag = 1;
      	continue;
    	}
    	visited_urls.insert(url);
		}

		//std::cout<<std::this_thread::get_id()<<":   "<<url<<"\n";

		CURL *curl = curl_easy_init();
		readBuffer.clear();

		if(curl) {
			CURLcode res;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			if(res!=CURLE_OK){
				std::cerr<<std::this_thread::get_id()<<":   error with getting html from: "<<url<<"\n";
				flag = 1;
				continue;
			}
	  }
		const char *readBufferChar = readBuffer.c_str();
		

		g_output = gumbo_parse(readBufferChar);
		std::vector<std::string> extracted_links = find_links(g_output->root);

		std::vector<std::string> temp_image_links = find_image_links(g_output->root);
		extracted_image_links.insert(extracted_image_links.end(), temp_image_links.begin(), temp_image_links.end());

		if(extracted_image_links.size()>100){
			std::lock_guard<std::mutex> lock(file_mutex);
			std::ofstream out;
			out.open(_FILENAME, std::ios_base::app);

		  for (const auto& image : extracted_image_links) {
    		out << image << std::endl;
  		}
  		out.close();

  		extracted_image_links.clear();
  		std::cout<<"added 100 urls :) \n";
		}




		if(extracted_links.size()>0){
			url = extracted_links.back();
			extracted_links.pop_back();
			flag = 0;
		}

		{
			std::lock_guard<std::mutex> lock(queue_mutex);
			for(const auto& link: extracted_links){
				url_queue.push(link);
			}
		}
	}
}



int main(void){
	std::unordered_set<std::string> visited_urls;
	std::queue<std::string> queue;
	queue.push("https://www.pexels.com/");
	queue.push("https://pixabay.com/");
	queue.push("https://www.freepik.com/");
	
	queue.push("https://www.shutterstock.com/");
	queue.push("https://unsplash.com/");
	queue.push("https://www.flickr.com/");


	std::mutex queue_mutex, list_mutex, file_mutex;

	std::vector<std::thread> threads;
	for(int i=0; i<6; ++i){
		threads.push_back(std::thread(crawler, std::ref(queue), std::ref(visited_urls), std::ref(queue_mutex), std::ref(list_mutex), std::ref(file_mutex)));

	}

	for (auto& thread : threads) {
		thread.join();
	}


	return 0;
}