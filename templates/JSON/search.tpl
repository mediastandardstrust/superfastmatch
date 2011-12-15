{{%AUTOESCAPE context="JSON"}}
"documents"   :{
                "metaData"  :{
                                "fields"      : [{{#FIELDS}}"{{FIELD}}"{{#FIELDS_separator}},{{/FIELDS_separator}}{{/FIELDS}}]
                },
                "rows"        : [
                                    {{#DOCUMENT}}
                                    {
                                        {{#META}}
                                        "{{KEY}}":"{{VALUE}}",
                                        {{/META}}
                                        "fragments" : [{{#FRAGMENT}}[{{FROM}},{{TO}},{{LENGTH}},{{HASH}}]{{#FRAGMENT_separator}},{{/FRAGMENT_separator}}{{/FRAGMENT}}]
                                    }{{#DOCUMENT_separator}},{{/DOCUMENT_separator}}
                                    {{/DOCUMENT}}
                                ]
},
{{#SOURCE}}
"text"      : "{{TEXT}}"
{{/SOURCE}}
