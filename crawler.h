#include "find_links.h"
#include "find_image_links.h"



int insert_db(auto myTable, std::vector<std::string> urls){
	int i = 0;
	for(auto s: urls){
		try{
			myTable.insert("url").values(s).execute();
			i++;
		}catch (mysqlx::abi2::r0::Error e){
			//std::cout<<e<<"\n";
		}
	}
	return i;
}



size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){//tak to musi wyglądać żeby curl to przyjął czyli chyba bufor, contents to to co curl otrzymuje,size to size nmemb to to ile jest elementów w buforze a userp to wskaźnik na to w czym chcemy mieć zapisane dane np później do std::string
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

void crawler(std::queue<std::string>& url_queue, std::unordered_set<std::string>& visited_urls, std::mutex& queue_mutex, std::mutex& list_mutex, std::string db_login, std::string db_password){
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

		if(extracted_image_links.size()>1000){
			mysqlx::Session sess("localhost", 33060, db_login, db_password);
			mysqlx::Schema db= sess.getSchema("img_url");

			auto myTable = db.getTable("urls");

			std::cout<<insert_db(myTable, extracted_image_links)<<"\n";
			extracted_image_links.clear();
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