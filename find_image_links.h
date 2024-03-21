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
    for(const auto& child_image : child_images){
    	if(child_image.compare(0, 7, "http://") == 0 || child_image.compare(0, 8, "https://") == 0){
    		links.push_back(child_image);
    	}
    }
  }

  return links;

}