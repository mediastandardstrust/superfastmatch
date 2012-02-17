    "documents" :{
        {{>DOCUMENTS}}
    }{{#SOURCE}},
    "text"      : "{{TEXT:o}}",
    {{#META}}
        "{{KEY:o}}": {{#NUMBER}}{{VALUE}}{{/NUMBER}}{{#STRING}}"{{VALUE:o}}"{{/STRING}}{{#META_separator}},{{/META_separator}}
    {{/META}}
    {{/SOURCE}}
