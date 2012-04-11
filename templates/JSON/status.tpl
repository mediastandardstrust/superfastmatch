    "dbs"       :[
                    {{#DB}}
                    {
                      {{#META}}
                      "{{KEY:o}}": {{#NUMBER}}{{VALUE}}{{/NUMBER}}{{#STRING}}"{{VALUE:o}}"{{/STRING}}{{#META_separator}},{{/META_separator}}
                      {{/META}}
                    }{{#DB_separator}},{{/DB_separator}}
                    {{/DB}}
                 ],
    "slots"     :[
                    {{#SLOT}}
                      "doc_count"           : {{DOC_COUNT}},
                      "hash_count"          : {{HASH_COUNT}},
                      "average_hashes"      : {{AVERAGE_HASHES}},
                      "average_doc_length"  : {{AVERAGE_DOC_LENGTH}}
                    {{/SLOT}}{{#SLOT_separator}},{{/SLOT_separator}}
                 ],
    "config"    :{
                    "window_size": {{WINDOW_SIZE}},
                    "whitespace_threshold": {{WHITE_SPACE_THRESHOLD}},
                    "hash_width":{{HASH_WIDTH}},
                    "slot_count":{{SLOT_COUNT}},
                    "whitespace_hash":{{WHITESPACE_HASH}}
                },
    "memory"    :{
                    "usage": {{MEMORY}},
                    "dump" : {{MEMORY_STATS}}
                 }
