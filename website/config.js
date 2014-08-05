{
    "service" : {
        "ip" : "0.0.0.0",
        "api" : "http",
        "port" : 8080,
        "disable_xpowered_by" : true
    },
    "http" : {
        "script_names" : [ "/website", "/rpc" ],
        "rewrite" : [
            { "regex" : "/", "pattern" : "/website" },
            { "regex" : "/files.*", "pattern" : "$0" },
            { "regex" : "/rpc.*", "pattern" : "/rpc$0" },
            { "regex" : ".*", "pattern" : "/website$0" }
        ]
    },
    "file_server": {
        "enable" : true,
        "allow_deflate" : true,
        "alias" : [
            {
                "url" : "/files",
                "path" : "files"
            }
        ]
    },
    "session" : {
        "expire" : "renew",
        "timeout" : 604800,
        "location" : "client",
        "client" : {
            "cbc" : "aes256",
            "cbc_key_file" : "cbc.txt",
            "hmac" : "sha256",
            "hmac_key_file" : "hmac.txt"
        }
    },
    "cache" : {
        "backend" : "thread_shared",
        "limit" : 200,
    }
}
