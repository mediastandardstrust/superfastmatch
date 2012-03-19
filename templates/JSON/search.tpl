    "documents" :{
        {{>DOCUMENTS}}
    }{{#SOURCE}},
    {{#META}}
    "{{KEY:o}}": {{#NUMBER}}{{VALUE}}{{/NUMBER}}{{#STRING}}"{{VALUE:o}}"{{/STRING}}{{#META_separator}},{{/META_separator}}
    {{/META}}
    "text"      : "{{TEXT:o}}"
    {{/SOURCE}}
