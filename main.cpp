#include <iostream>
#include <string>
#include <curl/curl.h>
#include <gumbo.h>
#include <vector>
#include <stack>
#include <unordered_set>
#include <cstring>


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

int main(void){
	std::unordered_set<std::string> visited_urls;
	std::stack<std::string> stack;
	stack.push("https://pl.wikipedia.org/wiki/Robot_internetowy");


	std::string readBuffer;

	while(!stack.empty()){
		std::string base_url = stack.top();
		stack.pop();

		if(visited_urls.find(base_url) != visited_urls.end()) {
      continue;
    }
    visited_urls.insert(base_url);		

		std::cout<<base_url<<"\n";
		CURL *curl = curl_easy_init();
		readBuffer.clear();




		if(curl) {
			CURLcode res;
			curl_easy_setopt(curl, CURLOPT_URL, base_url.c_str());//dodajemy adres, zawsze w setopt najpierw curl potem zależnie co chcemy zrobic
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);//podajemy naszą funkcję żebyśmy mogli z buforem zrobić to co chcemy
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);//no i dajemy wskaźnik na naszą zmienną z "odpowiedzią"
			res = curl_easy_perform(curl);//tu się wszystko robi okok
			curl_easy_cleanup(curl);//to zawsze na koniec ważne okok 
	  }

		const char *readBufferChar = readBuffer.c_str();

		GumboOutput* output = gumbo_parse(readBufferChar);
		std::vector<std::string> extracted_links = find_links(output->root);

		// for(auto s:extracted_links){
		// 	std::cout<<s<<"\n";
		// }	
		//make_absolute_url(base_url, extracted_links);

		for(const auto& link: extracted_links){
			stack.push(link);
		}
	}

	return 0;
}