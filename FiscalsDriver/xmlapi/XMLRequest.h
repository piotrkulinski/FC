#pragma once
#define GET_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); })

class XMLRequest {
	public:
		virtual XMLRequest& getType() {
			return *this;
		}
		virtual const char* getValue() {
			return nullptr;
		}
};

class XMLGetFeature : public XMLRequest {
public:
	virtual XMLGetFeature& getType() override {
		return *this;
	}
	XMLGetFeature(pugi::xml_node& el) {}
};

class XMLInfoCash : public XMLRequest {
public:
	virtual XMLInfoCash& getType() override {
		return *this;
	}
	XMLInfoCash(pugi::xml_node& el) {}
};

class XMLFiscalTicket : public XMLRequest {
protected:
	pugi::xml_node body;
public:
	virtual XMLFiscalTicket& getType() override {
		return *this;
	}
	XMLFiscalTicket(pugi::xml_node& el) {
		body = el;
	}
	virtual const char* getValue() {
	
		return (const char*)body.child_value();
	}
};

class XMLNonFiscalTicket : public XMLFiscalTicket {
public:
	virtual XMLNonFiscalTicket& getType() override {
		return *this;
	}
	XMLNonFiscalTicket(pugi::xml_node& el):XMLFiscalTicket(el) {
	}
};