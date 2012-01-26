        "metaData"  :{
                        "fields"      : [{{#FIELDS}}"{{FIELD:o}}"{{#FIELDS_separator}},{{/FIELDS_separator}}{{/FIELDS}}]
        },
        "rows"      :[
                        {{#DOCUMENT}}
                        {
                          {{#FRAGMENTS}}
                          "fragments" : [{{#FRAGMENT}}[{{FROM}},{{TO}},{{LENGTH}},{{HASH}}]{{#FRAGMENT_separator}},{{/FRAGMENT_separator}}{{/FRAGMENT}}],
                          {{/FRAGMENTS}}
                          {{#META}}
                          "{{KEY:o}}": {{#NUMBER}}{{VALUE}}{{/NUMBER}}{{#STRING}}"{{VALUE:o}}"{{/STRING}}{{#META_separator}},{{/META_separator}}
                          {{/META}}
                        }{{#DOCUMENT_separator}},{{/DOCUMENT_separator}}
                        {{/DOCUMENT}}
                     ]
