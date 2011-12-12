"metaData"  :{
                "fields"      : [{{#FIELDS}}"{{FIELD}}"{{#FIELDS_separator}},{{/FIELDS_separator}}{{/FIELDS}}]
},
"total" : {{TOTAL}},
"cursors": {    
                "current"   : "{{CURRENT}}",
                "first"     : "{{FIRST}}",
                "last"      : "{{LAST}}",
                "previous"  : "{{PREVIOUS}}",
                "next"      : "{{NEXT}}"
},
"rows"        : [
                    {{#DOCUMENT}}
                    {
                        {{#META}}"{{KEY}}":"{{VALUE}}"{{#META_separator}},{{/META_separator}}
                        {{/META}}
                    }{{#DOCUMENT_separator}},{{/DOCUMENT_separator}}{{/DOCUMENT}}
                ]