{{%AUTOESCAPE context="JSON"}}
"documents"   :{
                "metaData"  :{
                                "fields"      : ['doctype','docid','characters','title','group']
                },
                "rows"        : [
                                    {{#DOCUMENT}}
                                    {
                                        {{#META}}
                                        "{{KEY}}":"{{VALUE}}",
                                        {{/META}}
                                        "fragments" : [{{#FRAGMENT}}[{{FROM}},{{TO}},{{LENGTH}}]{{#FRAGMENT_separator}},{{/FRAGMENT_separator}}{{/FRAGMENT}}]
                                    }{{#DOCUMENT_separator}},{{/DOCUMENT_separator}}
                                    {{/DOCUMENT}}
                                ]
},
{{#SOURCE}}
"text"      : "{{TEXT}}"
{{/SOURCE}}
