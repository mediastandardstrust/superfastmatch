<table>
	<thead>
		<th>Doc Type</th>
		<th>Title</th>
		<th class="textColumn">Text</th>
		<th>From Position</th>
		<th>To Position</th>
		<th>Length</th>
	</thead>
	<tbody>
	{{#FRAGMENT}}
		<tr>
			<td><a href="/document/{{DOC_TYPE}}/">{{DOC_TYPE}}</a></td>
			<td><a href="/document/{{DOC_TYPE}}/{{DOC_ID}}">{{TITLE}}</a></td>
			<td>{{TEXT}}</td>
			<td>{{#LEFT_POSITIONS}}{{LEFT_POSITION}}{{#LEFT_POSITIONS_separator}}, {{/LEFT_POSITIONS_separator}}{{/LEFT_POSITIONS}}</td>
			<td>{{#RIGHT_POSITIONS}}{{RIGHT_POSITION}}{{#RIGHT_POSITIONS_separator}}, {{/RIGHT_POSITIONS_separator}}{{/RIGHT_POSITIONS}}</td>
			<td>{{LENGTH}}</td>
		</tr>
	{{/FRAGMENT}}
	</tbody>
</table>
