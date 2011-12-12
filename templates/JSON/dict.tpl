{{%AUTOESCAPE context="JSON"}}
{
  {{#PAIRS}}
  "{{KEY}}":{{VALUE}}
  {{#PAIRS_separator}},{{/PAIRS_separator}}
  {{/PAIRS}}
}
