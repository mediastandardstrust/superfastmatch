<h2><a href="/document/{{TO_DOC_TYPE}}/{{TO_DOC_ID}}">{{TO_TITLE}}</a></h2>
<h2><a href="/document/{{FROM_DOC_TYPE}}/{{FROM_DOC_ID}}">{{FROM_TITLE}}</a></h2>
<table>
	<thead>
		<th>Text</th>
		<th>From Position</th>
		<th>To Position</th>
		<th>Length</th>
	</thead>
	<tbody>
	{{#FRAGMENT}}
		<tr>
			<td>{{LEFT}}</td>
			<td>{{LEFT_POSITION}}</td>
			<td>{{RIGHT_POSITION}}</td>
			<td>{{LENGTH}}</td>
		</tr>
	{{/FRAGMENT}}
	</tbody>
</table>
