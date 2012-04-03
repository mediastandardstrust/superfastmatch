<!DOCTYPE html>
<html>
  <body>
    <style type="text/css">
    table.api{ 
      border-collapse:collapse;
    }
    table.api th,table.api td{
      border: 1px solid #000;
      padding: 4px;
    }
    </style>
    <h1>API</h1>
    <table class="api">
      <thead>
        <tr>
          <th>URL</th>
          <th>Query String</th>
          <th>Method</th>
          <th>Description</th>
          <th>Response Code</th>
          <th>Content Type</th>
        </tr>
      </thead>
      <tbody>
        {{#RESOURCE}}
        <tr>
          <td rowspan="{{RESPONSE_COUNT}}">{{URL:h}}</td>
          <td rowspan="{{RESPONSE_COUNT}}">{{#QUERY}}<a href="#{{ID}}">{{NAME}}</a>{{#QUERY_separator}}<br/>{{/QUERY_separator}}{{/QUERY}}</td>
          <td rowspan="{{RESPONSE_COUNT}}">{{METHOD:h}}</td>
          <td rowspan="{{RESPONSE_COUNT}}">{{DESCRIPTION:h}}</td>
          {{#RESPONSE}}
          <td>{{CODE:h}}</td>
          <td>{{CONTENT_TYPE:h}}</td>
          {{#RESPONSE_separator}}
        </tr>
        <tr>
          {{/RESPONSE_separator}}
          {{/RESPONSE}}
        </tr>
        {{/RESOURCE}}
      </tbody>
    </table>
    <h2>Parameters</h2>
    <table class="api">
      <thead>
        <tr>
          <th>Parameter</th>
          <th>Description</th>
        </tr>
      </thead>
      <tbody>
        {{#PARAMETER}}
        <tr>
          <td>{{TITLE:h}}</td>
          <td>{{DESCRIPTION:h}}</td>
        </tr>
        {{/PARAMETER}}
      </tbody>
    </table>
    <h2>Query String Parameters</h2>
    <table class="api">
      <thead>
        <tr>
          <th>Parameter</th>
          <th>Default Value</th>
          <th>Description</th>
        </tr>
      </thead>
      <tbody>
        {{#QUERIES}}
        <tr id="{{ID:h}}">
          <td>{{NAME:h}}</td>
          <td>"{{DEFAULT_VALUE:h}}"</td>
          <td>{{DESCRIPTION:h}}</td>
        </tr>
        {{/QUERIES}}
      </tbody>
    </table>
    
  </body>
</html>