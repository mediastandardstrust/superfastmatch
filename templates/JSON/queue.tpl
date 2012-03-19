    "total" : {{TOTAL}},
    "cursors": {    
                    "current"   : "{{CURRENT:o}}",
                    "first"     : "{{FIRST:o}}",
                    "last"      : "{{LAST:o}}",
                    "previous"  : "{{PREVIOUS:o}}",
                    "next"      : "{{NEXT:o}}"
    },
    "rows"      :[
                    {{#COMMAND}}
                    {{>DATA}}{{#COMMAND_separator}},{{/COMMAND_separator}}
                    {{/COMMAND}}
                 ]
