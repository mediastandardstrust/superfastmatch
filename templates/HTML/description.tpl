<!DOCTYPE html>
<html>
  <body>
    <style type="text/css">
    table{ 
      border-collapse:collapse;
    }
    th,td{
      border: 1px solid #000;
      padding: 4px;
    }
    </style>
    <h1>API</h1>
    <table>
      <thead>
        <tr>
          <th>URL</th>
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
    <table>
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
    
  </body>
</html>