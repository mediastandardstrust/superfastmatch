    "documents" :{
        {{>DOCUMENTS}}
    }{{#SOURCE}},
    {{#META}}
    "{{KEY:o}}": {{#NUMBER}}{{VALUE}}{{/NUMBER}}{{#STRING}}"{{VALUE:o}}"{{/STRING}},
    {{/META}}
    "text"      : "{{TEXT:o}}"
    {{/SOURCE}}
