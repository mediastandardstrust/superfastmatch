    "slots"     :[
                    {{#SLOT}}{
                      "slot_number"         : {{SLOT_NUMBER}},
                      "hash_count"          : {{HASH_COUNT}}
                    }{{#SLOT_separator}},{{/SLOT_separator}}{{/SLOT}}
                 ],
    "stats"     :{
                    "hash_count"          : {{HASH_COUNT}},
                    "doc_count"           : {{DOC_COUNT}},
                    "average_hashes"      : {{AVERAGE_HASHES}},
                    "average_doc_length"  : {{AVERAGE_DOC_LENGTH}},
                    "usage": {{MEMORY}},
                    "dump" : "{{MEMORY_STATS}}"
                },
    "config"    :{
                    "window_size": {{WINDOW_SIZE}},
                    "whitespace_threshold": {{WHITE_SPACE_THRESHOLD}},
                    "hash_width":{{HASH_WIDTH}},
                    "slot_count":{{SLOT_COUNT}},
                    "whitespace_hash":{{WHITE_SPACE_HASH}}
                },
    "dbs"       :[
                    {{#DB}}
                    {
                      {{#META}}
                      "{{KEY:o}}": {{#NUMBER}}{{VALUE}}{{/NUMBER}}{{#STRING}}"{{VALUE:o}}"{{/STRING}}{{#META_separator}},{{/META_separator}}
                      {{/META}}
                    }{{#DB_separator}},{{/DB_separator}}
                    {{/DB}}
                 ]

