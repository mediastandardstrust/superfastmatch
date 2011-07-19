mkdir fixtures
curl http://www.uni-weimar.de/medien/webis/research/corpora/pan-pc-11/pan11/pan11-plagiarism-external-competition-collection-2011-05-13.part1.rar -o fixtures/pan.part1.rar
curl http://www.uni-weimar.de/medien/webis/research/corpora/pan-pc-11/pan11/pan11-plagiarism-external-competition-collection-2011-05-13.part2.rar -o fixtures/pan.part2.rar
echo "Please UNRAR pan.part1.rar"
curl http://dl.dropbox.com/u/113479/sfm_text.zip -o fixtures/federal.zip
unzip fixtures/federal.zip -d fixtures/federal