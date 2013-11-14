//
//  main.cpp
//  cpp-stunnel
//
//  Created by Jeremy Xu on 13-11-14.
//  Copyright (c) 2013年 Jeremy Xu. All rights reserved.
//

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

namespace po = boost::program_options;
namespace asio = boost::asio;

using boost::asio::ip::tcp;
using namespace std;

class client {
private:
    string mode;
    string stunnelserverip;
    int stunnelserverport;
    int forwardport;
    string targetip;
    int targetport;
    boost::shared_ptr<asio::io_service> p_ios;
    boost::shared_ptr<tcp::socket> p_sock;
    
    void handle_resolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator) {
        if (!err) {
            // Attempt a connection to the first endpoint in the list. Each endpoint
            // will be tried until we successfully establish a connection.
            tcp::endpoint endpoint = *endpoint_iterator;
            p_sock->async_connect(endpoint, boost::bind(&client::handle_connect, this, boost::asio::placeholders::error, ++endpoint_iterator));
        }
        else {
            std::cout << "Error: " << err.message() << std::endl;
        }
    }
    
    void handle_connect(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator) {
        if (!err && p_sock->is_open()) {
            std::cout << "connect successfully!" << std::endl;
        } else if (endpoint_iterator != tcp::resolver::iterator()) {
            // The connection failed. Try the next endpoint in the list.
            p_sock->close();
            tcp::endpoint endpoint = *endpoint_iterator;
            p_sock->async_connect(endpoint, boost::bind(&client::handle_connect, this, boost::asio::placeholders::error, ++endpoint_iterator));
        } else {
            std::cout << "Error: " << err.message() << std::endl;
        }
    }

public:
    
    client():mode("R"), stunnelserverip("127.0.0.1"), stunnelserverport(8323), forwardport(8325), targetip("127.0.0.1"), targetport(22), p_ios(NULL), p_sock(NULL){
    }
    
    void parseArgs(int argc, const char * argv[]){
        po::options_description desc("stunnel-client allowed options");
        desc.add_options()
        ("mode", po::value<string>(), "tunnel mode [R | L]")
        ("stunnelserverip", po::value<string>(), "stunnel server ip")
        ("stunnelserverport", po::value<int>(), "stunnel server port")
        ("forwardport", po::value<int>(), "forward port")
        ("targetip", po::value<string>(), "target ip")
        ("targetport", po::value<int>(), "target port")
        ("help,h", "display this help")
        ("version,v", "show version")
        ;
        
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        
        if (vm.count("help")) {
            cout << desc << "\n";
            exit(0);
        }
        
        if (vm.count("version")) {
            cout << "stunnel-client v1.0.0" << "\n";
            exit(0);
        }
        
        if (vm.count("mode")) {
            mode = vm["mode"].as<string>();
        }
        
        if (vm.count("stunnelserverip")) {
            stunnelserverip = vm["stunnelserverip"].as<string>();
        }
        
        if (vm.count("stunnelserverport")) {
            stunnelserverport = vm["stunnelserverport"].as<int>();
        }
        
        if (vm.count("forwardport")) {
            forwardport = vm["forwardport"].as<int>();
        }
        
        if (vm.count("targetip")) {
            targetip = vm["targetip"].as<string>();
        }
        
        if (vm.count("targetport")) {
            targetport = vm["targetport"].as<int>();
        }
    }
    
    void setupTunnel() {
        p_ios.reset(new asio::io_service());
        p_sock.reset(new tcp::socket(*p_ios.get()));
        tcp::resolver targetresolver(*p_ios.get());
        tcp::resolver::query targetquery(stunnelserverip, boost::lexical_cast<std::string>(stunnelserverport));
        targetresolver.async_resolve(targetquery,
                                     boost::bind(&client::handle_resolve, this,
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::iterator));
        p_ios->run();
    }
};


int main(int argc, const char * argv[])
{
    try
    {
        client c;
        c.parseArgs(argc, argv);
        c.setupTunnel();
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << "\n";
    }
    return 0;
}

