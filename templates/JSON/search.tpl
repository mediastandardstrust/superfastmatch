{{%AUTOESCAPE context="JSON"}}
    "documents" :{
        {{>DOCUMENTS}}
    },
    {{#SOURCE}}
    "text"      : "{{TEXT:json_escape}}"
{{/SOURCE}}
