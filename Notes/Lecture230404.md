## Protocollo HTTP/1.0

The Hypertext Transfer Protocol (HTTP) is an application-level protocol with the lightness and speed necessary for distributed, collaborative, hypermedia information systems. It is a generic, stateless, object-oriented protocol which can be used for many tasks, such as name servers and distributed object management systems, through extension of its request methods (commands). A feature of HTTP is the typing of data representation, allowing systems to be built independently of the data being transferred.

## Request

A request message from a client to a server includes, within the first line of that message, the method to be applied to the resource, the identifier of the resource, and the protocol version in use.

```
Request 	= Simple-Request | Full-Request
Simple-Request 	= "GET" SP Request-URI CRLF
 Full-Request 	= Request-Line ; Section 5.1
 		*( General-Header ; Section 4.3
 		| Request-Header ; Section 5.2
 		| Entity-Header ) ; Section 7.1
 		CRLF
 			[ Entity-Body ] ; Section 7.2
```

If an HTTP/1.0 server receives a Simple-Request, it must respond with an HTTP/0.9 Simple-Response.

An HTTP/1.0 client capable of receiving a Full-Response should never generate a Simple-Request.

#### Full Request structure

The Request-Line begins with a method token, followed by the Request-URI and the protocol version, and ending with CRLF.

 `* ( General-Header | Request-Header | Entity-Header )`   tramite `*(` sappiamo che si ripetono e non vanno necessariamente messi in ordine. Non ho categorie di header, posso mettere gli header mischiati

Tutti gli header finiscono con il carattere speciale `CRLF`

Tra parentesi quadre abbiamo un Entity-Body (può esserci o può anche non esserci)

#### Request-Line

Com'è fatta la request line?

```
Request-Line = Method SP Request-URI SP HTTP-Version CRLF
```

Note that the difference between a Simple-Request and the Request-Line of a Full-Request is the presence of the HTTP-Version field and the availability of methods other than GET.

Abbiamo un token `Method` che ci indica il metodo to be performed on the resource identified by the `Request-URI` che è il percorso.

The Request-URI is a Uniform Resource Identifier (Section 3.2) and identifies the resource upon which to apply the request.

Alla fine della request line devo porre un token che mi identifica la versione dell' HTTP che sta usando. Non è stato possibile inserirla prima in quanto nella versione HTTP/0.9 non era stato pensato. E' infatti necessario mantenere sempre la retrocompatibilità.

#### Request Header Fields

The request header fields allow the client to pass additional information about the request, and about the client itself, to the server.

# Response

After receiving and interpreting a request message, a server responds in the form of an HTTP response message.

```
Response 	= Simple-Response | Full-Response

Simple-Response = [ Entity-Body ]

Full-Response 	= Status-Line
		*( General-Header | Response-Header | Entity-Header)
		CRLF
		[ Entity-Body ]
```

A Simple-Response should only be sent in response to an HTTP/0.9 Simple-Request or if the server only supports the more limited HTTP/0.9 protocol.

If a client sends an HTTP/1.0 Full-Request and receives a response that does not begin with a Status-Line, it should assume that the response is a Simple-Response and parse it accordingly.

#### Status-Line

The first digit of the Status-Code defines the class of response. The last two digits do not have any categorization role. There are 5 values for the first digit:

* 1xx: Informational - Not used, but reserved for future use
* 2xx: Success - The action was successfully received, understood, and accepted.
* 3xx: Redirection - Further action must be taken in order to complete the request
* 4xx: Client Error - The request contains bad syntax or cannot be fulfilled
* 5xx: Server Error - The server failed to fulfill an apparently valid request
