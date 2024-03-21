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
