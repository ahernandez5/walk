//modified by: Aurora Hernandez
//date:06/13/17
//purpose: HTTP, communication functions
//
//This program originated from the website: coding.debuntu.org
//Author: chantra
//Date: Sat 07/19/2008 - 19:23
//Usage:
//   $ gcc lab3prog.c -Wall -olab3prog
//   $ ./lab3prog
//   USAGE: prog host [page]
//          host: the website hostname. ex: coding.debuntu.org
//          page: the page to retrieve. ex: index.html, default: /
//
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <iostream>

//Generic web request class to contact any host and page
//using user agent and port provided
//Declared in both lab3http.cpp and walk.cpp because there is no common header

class WebRequest {
    // Functions are public only while testing then
    //go back to private if not testing
#ifdef UNIT_TEST
public:
#else
private:
#endif
    //internal functions
    int create_tcp_socket();
    char *get_ip(char *host); //unit test
    char *build_get_query(char *host, const char *page);
    void program_usage();
    std::string agent;
    int port;

public:
    WebRequest(std::string userAgent = "HTMLGET 1.0", int requestPort = 80);
    std::string request(std::string hostInput, std::string pageInput);
};

WebRequest::WebRequest(std::string userAgent, int requestPort)
: agent(userAgent), port(requestPort)
{
}

// it takes host and page parameters and returns data

std::string WebRequest::request(std::string hostInput, std::string pageInput)
{
    struct sockaddr_in *remote;
    int sock;
    int tmpres;
    char *ip;
    char *get;
    char buf[BUFSIZ + 1];

    // cstr now contains a c-string copy of str
    char page[pageInput.size()];
    strcpy(page, pageInput.c_str());
    char host[hostInput.size()];
    strcpy(host, hostInput.c_str());

    sock = create_tcp_socket();
    ip = get_ip(host);
    fprintf(stderr, "IP is %s\n", ip);
    remote = (struct sockaddr_in *) malloc(sizeof (struct sockaddr_in *));
    remote->sin_family = AF_INET;
    tmpres = inet_pton(AF_INET, ip, (void *) (&(remote->sin_addr.s_addr)));
    if (tmpres < 0) {
        perror("Can't set remote->sin_addr.s_addr");
        exit(1);
    } else if (tmpres == 0) {
        fprintf(stderr, "%s is not a valid IP address\n", ip);
        exit(1);
    }
    remote->sin_port = htons(port);

    if (connect(sock, (struct sockaddr *) remote,
            sizeof (struct sockaddr)) < 0) {
        perror("Could not connect");
        exit(1);
    }
    get = build_get_query(host, page);
    fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);

    //Send the query to the server
    //Changed type to unsigned int from int because
    //strlen returns an unsigned value
    unsigned int sent = 0;
    while (sent < strlen(get)) {
        tmpres = send(sock, get + sent, strlen(get) - sent, 0);
        if (tmpres == -1) {
            perror("send command, Can't send query");
            exit(1);
        }
        sent += tmpres;
    }
    //now it is time to receive the page
    memset(buf, 0, sizeof (buf));
    int htmlstart = 0;
    char * htmlcontent;
    std::string result;
    while ((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0) {
        if (htmlstart == 0) {
            /* Under certain conditions this will not work.
             * If the \r\n\r\n part is splitted into two messages
             * it will fail to detect the beginning of HTML content
             */
            htmlcontent = strstr(buf, "\r\n\r\n");
            if (htmlcontent != NULL) {
                htmlstart = 1;
                htmlcontent += 4;
            }
        } else {
            htmlcontent = buf;
        }
        if (htmlstart) {
            fprintf(stdout, htmlcontent);
            //copy htmlcontent to result string
            //because it's local scope
            result = htmlcontent;
        }

        memset(buf, 0, tmpres);
    }
    if (tmpres < 0) {
        perror("Error receiving data");
    }

    free(get);
    free(remote);
    free(ip);
    close(sock);
    return result;
}

int WebRequest::create_tcp_socket()
{
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("Can't create TCP socket");
        exit(1);
    }
    return sock;
}

char* WebRequest::get_ip(char *host)
{
    struct hostent *hent;
    //ip address format  123.123.123.123
    //Changed iplen from 15 to 16 because it did not
    //have enough memory to allocate the address in inet_ntop
    //IP address requires 16 bits, to be able to represent all versions of 
    //IP addresses
    int iplen = 16;
    //changed to allocate just iplen bytes
    char *ip = (char *) malloc(iplen);
    //changed to memset just iplen bytes  
    memset(ip, 0, iplen);
    if ((hent = gethostbyname(host)) == NULL) {
        herror("Can't get IP host by name");
        exit(1);
    }
    if (inet_ntop(AF_INET, (void *) hent->h_addr_list[0], ip, iplen) == NULL) {
        perror("Can't resolve host with inet_ntop");
        exit(1);
    }
    return ip;
}

//Changed the parameter char* page to const char* page to reflect
//the prototype

char* WebRequest::build_get_query(char *host, const char *page)
{
    char *query;
    //Changed type from char* to const char* to fix deprecated error
    const char *getpage = page;
    //Checked 
    const char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
    if (getpage[0] == '/') {
        getpage = getpage + 1;
        fprintf(stderr, "Removing leading \"/\", converting %s to %s\n", page,
                getpage);
    }
    // -5 is to consider the %s %s %s in tpl and the ending \0
    query = (char *) malloc(strlen(host) + strlen(getpage) + agent.size()
            + strlen(tpl) - 5);
    sprintf(query, tpl, getpage, host, agent.c_str());
    return query;
}

#ifdef UNIT_TEST

void unit_web_ip(){
    WebRequest webrequest;
    char host[] = "localhost";
    char* result = webrequest.get_ip(host);
    char expected[] = "127.0.0.1";
    std:: cout << "Result is: " << result << "\n";
    std:: cout << "Expected was " << expected << "\n";
    if(strcmp(result, expected) == 0) {
        std:: cout << "Test succeeded\n";
    } else {
        std:: cout << "Test failed\n";
    }

}
//unit_web_ip();

#endif //UNIT_WEB_TEST 

