#include "BasicController.h"

/*
https://learn.microsoft.com/pl-pl/dotnet/framework/wcf/feature-details/how-to-configure-a-port-with-an-ssl-certificate
W systemie Windows wystarczy zmieniæ protokó³ z „http” na „https” w adresie URL, 
pod warunkiem, ¿e do³¹czono certyfikat serwera SSL do portu u¿ywanego przez 
serwer na komputerze, na którym dzia³a serwer. 
Zobacz tutaj, aby uzyskaæ bardzo dobry opis i szczegó³owe kroki zwi¹zane z t¹ procedur¹.
netsh http show sslcert

http://ib-krajewski.blogspot.com/2015/09/https-support-for-casablanca-server-and.html

-- Wi¹zanie certyfikatu SSL z numerem portu
netsh http add sslcert ipport=0.0.0.0:8085 certhash=0000000000003ed9cd0c315bbb6dc1c08da5e6 appid={00112233-4455-6677-8899-AABBCCDDEEFF}


--Wi¹zanie certyfikatu SSL z numerem portu i obs³ug¹ certyfikatów klienta
netsh http add sslcert ipport=0.0.0.0:8085 certhash=0000000000003ed9cd0c315bbb6dc1c08da5e6 appid={00112233-4455-6677-8899-AABBCCDDEEFF} clientcertnegotiation=enable

--Usuwanie certyfikatu SSL z numeru portu
Netsh http delete sslcert ipport=0.0.0.0:8085

-------------------------------------------------------------------
//linux
	* //define CPPREST_FORCE_HTTP_LISTENER_ASIO
	web::http::experimental::listener::http_listener_config conf;
	conf.set_ssl_context_callback(
		[](boost::asio::ssl::context& ctx)
		{
			ctx.set_options(boost::asio::ssl::context::default_workarounds);
			// Password callback needs to be set before setting cert and key.
			ctx.set_password_callback(
			[](std::size_t max_length, boost::asio::ssl::context::password_purpose purpose)
			{
				return "password";
			});
			ctx.use_certificate_file("cert.pem", boost::asio::ssl::context::pem);
			ctx.use_private_key_file("key.pem", boost::asio::ssl::context::pem);
			ctx.use_certificate_chain_file("chain.pem");
		});
*/

BasicController::BasicController(const std::wstring& naddress, const std::wstring& nport) {
    
    std::wstring schema{L"http"};
    this->endpointBuilder.set_host(naddress);
    this->endpointBuilder.set_port(nport);
    this->endpointBuilder.set_scheme(schema);
}
BasicController::~BasicController() {
}
void BasicController::setEndpoint(const utility::string_t& mount_point)
{
    endpointBuilder.set_path(mount_point);
    _listener = http_listener(endpointBuilder.to_uri());
}

pplx::task<void> BasicController::accept() {
    initRestOpHandlers();
    return _listener.open();
}
pplx::task<void> BasicController::shutdown() {
    return _listener.close();
}

std::vector<utility::string_t> BasicController::requestPath(const http_request& message)
{
    auto relativePath = uri::decode(message.relative_uri().path());
    return uri::split_path(relativePath);
}
