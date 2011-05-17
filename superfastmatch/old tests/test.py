#!/usr/bin/python
# -*- coding: utf-8 -*-
import superfastmatch.fast
import fastmatch
import unittest 
import os
import urllib
import codecs
from operator import itemgetter

def printresults(matches,s1,s2):
    for match in matches:
        print '(%s,%s,%s)"%s","%s"' %(match[0],match[1],match[2],s1[match[0]:match[0]+match[2]].encode('utf-8'),s2[match[1]:match[1]+match[2]].encode('utf-8'))

class TestSuperFastMatch(unittest.TestCase):
    def test_equal(self):
        s1 = "1234"
        s2 = "1234"
        matches = fastmatch.match(s1,s2,2)
        self.assertEqual(len(matches),1)
        printresults(matches,s1,s2)
    
    def test_singleword(self):
        s1 = "fantastic"
        s2 = "fantastic123"
        matches = fastmatch.match(s1,s2,2)
        self.assertEqual(len(matches),1)
        printresults(matches,s1,s2)
    
    def test_bad_window_size(self):
        self.assertRaises(TypeError, fastmatch.match,('test','a test',1))
    
    def test_ending(self):
        s1 = "fantastic blank 12345"
        s2 = "fantastic12345"
        matches = fastmatch.match(s1,s2,2)
        printresults(matches,s1,s2)
        self.assertEqual(len(matches),3)
    
    def test_simple(self):
        s1 = "I'm a test with a section that will be repeated repeated testingly testingly"
        s2 = "I will be repeated repeated testingly 123 testingly"
        matches = fastmatch.match(s1,s2,6)
        printresults(matches,s1,s2)
        self.assertEqual(len(matches),6)
    
    def test_short_unicode(self):
        s1 = u"This is “unicode”"
        s2 = u"unicode I am"
        matches = fastmatch.match(s1,s2,2)
        printresults(matches,s1,s2)
    
    def test_unicode(self):
        s1 = u"From time to time this submerged or latent theater in becomes almost overt. It is close to the surface in Hamlet’s pretense of madness, the “antic disposition” he puts on to protect himself and prevent his antagonists from plucking out the heart of his mystery. It is even closer to the surface when Hamlet enters his mother’s room and holds up, side by side, the pictures of the two kings, Old Hamlet and Claudius, and proceeds to describe for her the true nature of the choice she has made, presenting truth by means of a show. Similarly, when he leaps into the open grave at Ophelia’s funeral, ranting in high heroic terms, he is acting out for Laertes, and perhaps for himself as well, the folly of excessive, melodramatic expressions of grief."
        s2 = u"Almost all of Shakespeare’s Hamlet can be understood as a play about acting and the theater. For example, there is Hamlet’s pretense of madness, the “antic disposition” that he puts on to protect himself and prevent his antagonists from plucking out the heart of his mystery. When Hamlet enters his mother’s room, he holds up, side by side, the pictures of the two kings, Old Hamlet and Claudius, and proceeds to describe for her the true nature of the choice she has made, presenting truth by means of a show. Similarly, when he leaps into the open grave at Ophelia’s funeral, ranting in high heroic terms, he is acting out for Laertes, and perhaps for himself as well, the folly of excessive, melodramatic expressions of grief."
        matches = fastmatch.match(s1.lower(),s2.lower(),8)
        printresults(matches,s1,s2)
        self.assertEqual(len(matches),19)
    
    def test_bug_from_site(self):
        s1=u'\nThe body of a man has been recovered from the River Tweed near Coldstream following a police operation.\nIt follows an eight-day river search for missing man Philip Shoemaker, 51, from Kelso by Lothian and Borders Police.\nRescue units and a helicopter joined search teams in the area following several reports received from members of the public.\nNo formal identification has been made of the man found in the river.\n'
        s2=u'\nAt 12.26pm on Monday, April 23, reports were received of a body lying on the mud flats between the Redheugh Bridge and King Edward Bridge over the River Tyne.\nNorthumbria Police Marine Unit attended and discovered the body of an elderly man.\nThe body was recovered but has not yet been formally identified.\xa0 There are no suspicious circumstances regarding this incident.'
        matches = fastmatch.match(s1.lower(),s2.lower(),15)
        print matches
        printresults(matches,s1,s2)
        self.assertEqual(len(matches),0)
    
    def test_long_one(self):
        s1=u"""at 12.40am on Sunday, August 5, police were called to the Cashmere nightclub in Berwick after the CCTV operator at Berwick police station saw that signs of a dispute between two factions who had been refused admission to the club by the doorstaff. A total of four officers were called to respond. When the first two arrived at the scene both factions turned on them and assaulted them. One officer was punched to the ground and continued to be attacked as he lay unconscious. The second officer received face and body injuries. By this time two more officers arrived at the scene and were also assaulted. More resources were called out, including officers from Alnwick, the area support group from Hexham, dog handlers and the North East Air Support Unit. Officers from Lothian and Borders police also gave assistance at the scene. The doorstaff also gave valuable assistance to bring the situation under control. Four men were arrested. The four, aged 18, 33, 34 and 44, are all from Berwick and were arrested on suspicion of causing grievous bodily harm, violent disorder and drunk and disorderly. The first four officers on the scene, who are all male,  work at Northumberland area command and are based in Berwick. They were all wearing regulation body armour. Their ages range from 30 to 42.  The officers  who were taken to hospital, one aged 39 and one aged 42, continue to be detained. Both are experienced officers with Northumbria, one with 11 years' service and the other 17. Their injuries are not considered life threatening and both are in a stable condition. A total of four other officers - three men and a woman -  will receive medical attention from the force's Occupational Health Unit. Supt Gordon Milward, Northumberland area command, said: "First of all I must stress that this incident was totally out of character for Berwick, where disorder like this is extremely uncommon. We still don't know why what started as a dispute outside a club should have escalated in this way. "Attacks against police officers are totally unacceptable and there is no doubt that without the assistance of the doorstaff the outcome could have been even more serious. Fortunately, the officers' injuries do not appear to be as serious as was first thought, but any attack against a police officer is one too many. "We are leaving no stone unturned in the investigation to bring those responsible to justice. We are carrying out an extensive review of all CCTV coverage in the town centre overnight in an effort to trace anyone acting suspiciously in and around the Golden Square area. It's highly likely that more arrests will be made. We are also increasing reassurance patrols around Berwick." Anyone with information is asked to contact Northumbria Police on 08456 043 043 or call Crimestoppers."""
        s2=u"""Two police officers are in hospital after they were attacked outside a Northumberland nightclub. One of the men, who had been called to a dispute at the Cashmere club in Berwick, was punched to the ground and attacked again as he lay unconscious. The second officer suffered facial and body injuries in the clash, believed to have been sparked when two groups of people were refused admission. Four Berwick men, aged 18 to 44, have been arrested. They are being questioned over the incident, which happened in the early hours of Sunday. The two officers were taken to Wansbeck General Hospital, where their condition is described as stable. Police said the disturbance broke out after two separate groups of revellers were refused admission to the nightclub. Four police officers were called to the scene by CCTV operators and the two groups joined force to attack the officers when they arrived. Reinforcements were called out from Alnwick, and four other police officers were hurt in the clashes. The club's door staff have been praised for helping to bring the incident under control. 'More arrests' Supt Gordon Milward, of Northumbria Police, said: "We still don't know why what started as a dispute outside a club should have escalated in this way. "Attacks against police officers are totally unacceptable and there is no doubt that without the assistance of the door staff the outcome could have been even more serious. "We are leaving no stone unturned in the investigation to bring those responsible to justice. "It's highly likely that more arrests will be made." Officers are now reviewing CCTV coverage in the town centre as part of the investigation."""
        matches = fastmatch.match(s1.lower(),s2.lower(),15)
        printresults(matches,s1,s2)
        self.assertEqual(len(matches),31)
    
    def test_long_nearly_same(self):
        s1=u"Arab foreign ministers meeting in Cairo to forge a unified response to the Israeli offensive against Gaza have asked the Security Council to convene to issue a resolution compelling Israel to “cease immediately [its] aggression.”A delegation headed by Prince Saudi Al Faisal, the Saudi foreign minister, will press the Arab case in New York.The Cairo meeting also discussed calls by Qatar, Syria and Yemen for an emergency Arab summit but the ministers said they had decided to defer a decision until they had tested what could be achieved through the Security Council.The resolutions adopted at the Arab League reflect the positions of regional heavyweights Egypt and Saudi Arabia - American allies who regard the Palestinian group Hamas which controls Gaza as an obstacle to efforts to revive a peace process with Israel.The two countries are anxious to avoid a summit where they are likely to come under pressure from Syria - a Hamas backer - to take steps that might satisfy public opinion but place them at odds with their American friends.The daily images of Israel’s bombardment of Gaza, and the non-stop flow of pictures of dead and injured children on Arab television screens have angered the Arab public and led to widespread condemnation of regional leaders for their inability to defend the Palestinians.Demonstrators in Egypt and elsewhere have demanded that Cairo and Amman break off ties with Israel and expel its ambassadors. There have also been calls for Saudi Arabia to withdraw the initiative it launched in 2002, offering Israel normal relations with all Arab countries in return to its withdrawal to 1967 lines.Egypt in particular has been the target of enormous popular anger for its insistence on sealing its border with Gaza since Hamas seized control of the territory eighteen months ago. The Rafah crossing into Egypt is Gaza’s only outlet to the outside world which does not go through Israel.But Cairo has been deeply rattled by the gains made by the Islamists of Hamas at the expense of their rivals in the Palestinian Authority. Hamas has strong ties with the outlawed Muslim Brotherhood opposition in Egypt. The group does not recognise Israel and calls for armed resistance against it, rejecting the Middle East peace process - a centre piece of Egypt’s foreign policy.Syria reportedly tried to press for the opening of the Rafah crossing at the Arab League meeting, but was turned down. In recent days Cairo has allowed some humanitarian aid to go into Gaza and Palestinians injured in the bombings to cross into Egypt for treatment. But the Egyptian authorities insist the border should remain closed until the Palestinian Authority ousted from Gaza by Hamas is able to run the crossing. Since the start of the Gaza offensive, Arab satellite channels such as al-Jazeera have given much airtime to critics of Egypt who have lambasted the country for accusing Hamas of provoking the Israeli attacks and for receiving Tzipi Livni, the Israeli foreign minister, in Cairo just a day before the launch of the military campaign.In an apparent attempt to assuage Cairo’s hurt feelings, the resolution issued by the Arab foreign ministers “praised the huge efforts made by the Arab Republic of Egypt to support the Palestinian people and to reconcile the Palestinian factions in order to achieve national unity.”"
        s2=u"Arab foreign ministers meeting in Cairo to frge a unified response to the Israeli offensive against Gaza have asked the Security Council to convene to issue a resolution compelling Israel to “cease immediately [its] aggression.”A delegation headed by Prince Saudi Al Faisal, the Saudi foreign minister, will press the Arab case in New York.The Cairo meeting also discussed calls by Qatar, Syria and Yemen for an emergency Arab summit but the ministers said they had decided to defer a decision until they had tested what could be achieved through the Security Council.The resolutions adopted at the Arab League reflect the positions of regional heavyweights Egypt and Saudi Arabia - American allies who regard the Palestinian group Hamas which controls Gaza as an obstacle to efforts to revive a peace process with Israel.The two countries are anxious to avoid a summit where they are likely to come under pressure from Syria - a Hamas backer - to take steps that might satisfy public opinion but place them at odds with their American friends.The daily images of Israel’s bombardment of Gaza, and the non-stop flow of pictures of dead and injured children on Arab television screens have angered the Arab public and led to widespread condemnation of regional leaders for their inability to defend the Palestinians.Demonstrators in Egypt and elsewhere have demanded that Cairo and Amman break off ties with Israel and expel its ambassadors. There have also been calls for Saudi Arabia to withdraw the initiative it launched in 2002, offering Israel normal relations with all Arab countries in return to its withdrawal to 1967 lines.Egypt in particular has been the target of enormous popular anger for its insistence on sealing its border with Gaza since Hamas seized control of the territory eighteen months ago. The Rafah crossing into Egypt is Gaza’s only outlet to the outside world which does not go through Israel.But Cairo has been deeply rattled by the gains made by the Islamists of Hamas at the expense of their rivals in the Palestinian Authority. Hamas has strong ties with the outlawed Muslim Brotherhood opposition in Egypt. The group does not recognise Israel and calls for armed resistance against it, rejecting the Middle East peace process - a centre piece of Egypt’s foreign policy.Syria reportedly tried to press for the opening of the Rafah crossing at the Arab League meeting, but was turned down. In recent days Cairo has allowed some humanitarian aid to go into Gaza and Palestinians injured in the bombings to cross into Egypt for treatment. But the Egyptian authorities insist the border should remain closed until the Palestinian Authority ousted from Gaza by Hamas is able to run the crossing. Since the start of the Gaza offensive, Arab satellite channels such as al-Jazeera have given much airtime to critics of Egypt who have lambasted the country for accusing Hamas of provoking the Israeli attacks and for receiving Tzipi Livni, the Israeli foreign minister, in Cairo just a day before the launch of the military campaign.In an apparent attempt to assuage Cairo’s hurt feelings, the resolution issued by the Arab foreign ministers “praised the huge efforts made by the Arab Republic of Egypt to support the Palestinian people and to reconcile the Palestinian factions in order to achieve natinal unity.”"
        matches = fastmatch.match(s1.lower(),s2.lower(),15)
        printresults(matches,s1,s2)
        self.assertEqual(len(matches),52) #Test of HARD_LIMIT
        
    def test_hashes(self):
       s = "I need to be hashed"
       hashes = superfastmatch.fast.hashes(s,15)
       print hashes
       self.assertEqual(len(hashes),5)
       
    def test_hashwindow(self):
       s = "hash"
       hashes = superfastmatch.fast.hashes(s,2)
       for i in range(0,len(hashes)):
           print s[i:i+2],hashes[i]
       self.assertEqual(len(hashes),3)
    
    def test_24bit_hashes(self):
       hashes = superfastmatch.fast.hashes(os.urandom(8192),15,24)
       # print hashes
       print (max(hashes)<(2**24-1))
       self.assertTrue((max(hashes)<(2**24-1)))
    
    def test_32bit_hashes(self):
       hashes = superfastmatch.fast.hashes(os.urandom(8192),15,32)
       # print hashes
       print (max(hashes)<(2**32-1))
       self.assertTrue((max(hashes)<(2**32-1)))
    
    def test_32bit_hashes_with_bounds(self):
       upper = (2**32)/2
       lower = (2**32)/4
       NUM_HASHES = 8192
       hashes = superfastmatch.fast.hashes(os.urandom(NUM_HASHES),15,32,lower,upper)
       # print hashes
       print "Input: %s Output: %s Max: %s Min: %s Upper:%s Lower %s" % (NUM_HASHES,len(hashes),max(hashes),min(hashes),upper,lower)
       self.assertTrue((min(hashes)>=lower))
       self.assertTrue((max(hashes)<=upper))
    
    def test_long_overlaps(self):
           s2 = u"""Raoul Moat has enough local knowledge and survival skills to hide out in the
             woods for days, a former lover said.
           The fugitive, who in a rambling letter to police admitted to being a "killer
             and a maniac", loves camping and fishing and spent many weekends in the
             Northumberland countryside as a younger man.
           His written declaration of war leads police to believe he would rather die
             taking them on than give himself up peacefully.
           The 37-year-old even described himself feeling like The Hulk, when "it
             takes over and it's more than anger and it happens only when I'm hurt, and
             this time I was really hurt."
           But a different picture was painted by waitress Yvette Foreman, 35, who dated
             the former nightclub doorman more than a decade ago.
           She said: "He was a lovely lad back then. He had long blonde hair in a
             ponytail, which he wore plaited with loads of little coloured beads.
           "He'd come up to stay from Newcastle for the weekend and we'd go off to
             the woods.
           "He was daft as a brush, a real practical joker. He once soaked my dad
             with water."
           It was during their romance that he got to know the Rothbury area well.
           She said: "We'd go camping and fishing here loads when we were younger,
             almost every weekend, and he knows the woods and hills like the back of his
             hand.
           "There's so much woodland and so many little nooks and crags out there I
             think he could hide out for days."
           Only once did he reveal his volatile streak, and his enjoyment of violence.
           He wrote to her a vicious fantasy in which he described plans to batter a
             rival with a baseball bat.
           She recalled: "He was saying how he and his mates were going to take a
             bat to somebody because they'd fallen out with somebody.
           "I never really paid it any attention until now."
           Miss Foreman said the burly bouncer's apparent steroid abuse could be to
             blame.
           Relatives of his former partner Samantha Stobbart said he was a vain thug,
             prone to horrific outbursts of violence fuelled by jealousy.
           Her half-sister Kelly said: "He's a nutter and he's definitely not going
             to give himself up."
           He has been arrested 12 separate times and charged with seven different
             offences during that time, police confirmed.
           He went on trial at Newcastle Crown Court charged with possessing a samurai
             sword and a knuckle duster five years ago.
           Despite boasting to the jury about the best use of a knuckle-duster and his
             brawling skills, he was acquitted of both counts.
           His first conviction - despite known links to the Tyneside underworld - came
             only this year when he was jailed for 18 weeks for assaulting a child.
           He was brought up in the West End of Newcastle by his grandmother, close to
             where his father and French-born mother lived.
           Police were continuing to search for Moat in the area around Rothbury.
           Although poor weather conditions limited air operations overnight armed
             officers supporting specialist search teams conducted a thorough search of
             the rural location, police said.
           The search is continuing today as officers believe Moat may still be in the
             area.
           Temporary Deputy Chief Constable Jim Campbell said: "We were aware a search of
             this open farmland area with many abandoned buildings and dense areas of
             woodland posed a particular challenge.
           "Despite a large-scale search conducted by specialist teams we have not yet
             located Moat.
           "Officers are continuing the search today and are renewing their appeals for
             help from the public in locating this man.
           "Our message remains the same - if you see this man who has a distinctive
             appearance call the police straight away.
           "We have resources available throughout the force area who can be deployed.
           "We want to reassure residents in the Rothbury area that despite this activity
             it is still very much 'business as usual' for local residents.
           "Schools in Rothbury will be fully open on Wednesday and police resources will
             be in place to reassure parents that they can safely take their children to
             and from school.
           "Local residents are being encouraged to go about their normal business.
           "The stringent two mile exclusion zone was downgraded overnight. "This means
             that while police officers will still be patrolling the area, residents and
             visitors, and their vehicles, may be checked by officers coming into the
             town and also when they leave.
           "We will continue to keep a large and highly visible police presence in the
             town of Rothbury for as long as necessary while the search continues.
           "We do need help from the public to be vigilant and to report any suspicious
             incidents to us immediately.
           "Then we can respond and take any necessary action.
           "We recognise that this large scale police presence has interrupted normal
             life for many people and we want to reassure residents that we are doing all
             we can to bring this investigation to a conclusion." """
           s1 = u"""Police are continuing to search for Raoul Thomas Moat in the area around Rothbury in Northumberland.
           Although poor weather conditions limited air operations overnight armed officers supporting specialist search teams conducted a thorough search of the rural location.
           The search continues today as officers believe that Moat may still be in the area.
           Temporary Deputy Chief Constable Jim Campbell said: "We were aware a search of this open farmland area with many abandoned buildings and dense areas of woodland posed a particular challenge. Despite a large scale search conducted by specialist teams we have not yet located Moat.
           "Officers are continuing the search today and are renewing their appeals for help from the public in locating this man.
           "Our message remains the same - if you see this man who has a distinctive appearance call the police straightaway.  We have resources available throughout the force area who can be deployed.
           "We want to reassure residents in the Rothbury area that despite this activity it is still very much 'business as usual' for local residents. Schools in Rothbury will be fully open on Wednesday and police resources will be in place to reassure parents that they can safely take their children to and from school. Local residents are being encouraged to go about their normal business.
           "The stringent two mile exclusion zone was downgraded overnight.  This means that while police officers will still be patrolling the area, residents and visitors, and their vehicles, may be checked by officers coming into the town and also when they leave.
           "We will continue to keep a large and highly visible police presence in the town of Rothbury for as long as necessary while the search continues.
           "We do need help from the public to be vigilant and to report any suspicious incidents to us immediately. Then we can respond and take any necessary action.
           "We recognise that this large scale police presence has interrupted normal life for many people and we want to reassure residents that we are doing all we can to bring this investigation to a conclusion."
           """
           matches = fastmatch.match(s1.lower(),s2.lower(),15)
           printresults(matches,s1,s2)
           self.assertEqual(len(matches),86)
        
    def test_html_junk(self):
        s1 = u"""//Off "); }; function performAutoRefresh() { if(checkAutoRefreshCookie()) { startTheTimer(); }; }; function setSwitchDefaultOn() { jQ("span#auto-on a").replaceWith("On"); jQ("span#auto-off strong").replaceWith("Off"); jQ("div#helper").addClass("image-loading"); }; function setSwitchDefaultOff() { jQ("span#auto-off a").replaceWith("Off"); jQ("span#auto-on strong").replaceWith("On"); jQ("div#helper").removeAttr("class"); }; function startTheTimer() { jQ(document).everyTime(60000, 'initialAutoRefresh', function() { var refreshCacheBusting = parseInt((Math.random() * 100000), 10); jQ("#"+targettedArea).load(window.location+'?refresh=true&_='+refreshCacheBusting+' #'+targettedArea+' > *'); }); }; function OnOffHandlers() { jQ('#auto-on').mousedown( function(){ jQ.cookie('autoRefresh', 'on'); setSwitchDefaultOn(); performAutoRefresh(); return false; }); jQ('#auto-off').mousedown( function(){ jQ.cookie('autoRefresh', 'off'); setSwitchDefaultOff(); jQ(document).stopTime('initialAutoRefresh'); return false; }); }; // ]]> To receive updated content, refresh the page (F5 for a web browser). Raoul Moat, aged six. Photograph: JK PRESS 8.38am: Family photos have been released of Moat as a boy. The Sun newspaper's notes the change from "a cherubic ginger haired boy" to "psycho commando". It also dummies up an image of how Moat might look now based on the police description of his size and what he is believed to be wearing. 8.18am: Nothumbria Police is refusing to confirm or deny the Mail's report that the SAS has joined the hunt. Sky News claimed they had denied it, but when I called a couple of minutes ago a spokesman said: "We are not confirming or denying it. We are not commenting on it." He added there were, as yet, no plans for a press conference this morning. 8.07am: "Raoul son, please this has to stop, it's gone on far too long," says Samantha Stobbart's father, Paul in a video message posted by Northumbria Police last night. 7.58am: The search continues around the Rothbury area. This clickable map shows the area and the key discoveries so far. View Rothbury in a larger map 7.49am: Here are the main development overnight. • Raoul Moat, who is believed to have shot his girlfriend, fatally shot her new partner and seriously wounded a policeman, continues to evade capture . Northumbria police still think he is hiding out in the countryside in the Rothbury area. Temporary deputy chief constable Jim Campbell said today: The searches in this area have proved a particular challenge due to the open farmland and dense woodland and officers are continuing in their efforts today. I'd like to reassure the public that we are doing everything possible to locate Moat and bring this investigation to a conclusion. Although much of the inquiry centres on Rothbury the events in this area are just one part of a complex investigation and activity continues across the force. • The SAS have joined in the manhunt, according to the Daily Mail. Citing sources, the paper said the "special forces excel at working in the dark, with heat-seeking equipment which can work over huge ranges" . • Moat's mother, Josephine Healey, said he would be "better off dead" . Speaking to the Daily Telegraph she said: "I feel like he hasn't been my son since he was 19 years old. He now has a totally different character, attitude and manner. Now when I see him I don't recognise him at all ... If I was to make an appeal I would say he would be better dead." • Two men arrested during the manhunt have been charged with conspiracy to commit murder. Karl Ness and Qhuram Awan are due to appear at Newcastle magistrates court at 10am today."""
        s2 = u"""Police are continuing to search for Raoul Thomas Moat in the area around Rothbury in Northumberland.
        Although poor weather conditions limited air operations overnight armed officers supporting specialist search teams conducted a thorough search of the rural location.
        The search continues today as officers believe that Moat may still be in the area.
        Temporary Deputy Chief Constable Jim Campbell said: "We were aware a search of this open farmland area with many abandoned buildings and dense areas of woodland posed a particular challenge. Despite a large scale search conducted by specialist teams we have not yet located Moat.
        "Officers are continuing the search today and are renewing their appeals for help from the public in locating this man.
        "Our message remains the same - if you see this man who has a distinctive appearance call the police straightaway.  We have resources available throughout the force area who can be deployed.
        "We want to reassure residents in the Rothbury area that despite this activity it is still very much 'business as usual' for local residents. Schools in Rothbury will be fully open on Wednesday and police resources will be in place to reassure parents that they can safely take their children to and from school. Local residents are being encouraged to go about their normal business.
        "The stringent two mile exclusion zone was downgraded overnight.  This means that while police officers will still be patrolling the area, residents and visitors, and their vehicles, may be checked by officers coming into the town and also when they leave.
        "We will continue to keep a large and highly visible police presence in the town of Rothbury for as long as necessary while the search continues.
        "We do need help from the public to be vigilant and to report any suspicious incidents to us immediately. Then we can respond and take any necessary action.
        "We recognise that this large scale police presence has interrupted normal life for many people and we want to reassure residents that we are doing all we can to bring this investigation to a conclusion."
        """
        matches = fastmatch.match(s1.lower(),s2.lower(),15)
        printresults(matches,s1,s2)
        self.assertEqual(len(matches),12)
    
    def test_very_long(self):
        s1 = codecs.open('long.txt',encoding='utf-8').read()
        s2 = u"""The Secretary of State for Culture, Media and Sport, The Rt Hon James Purnell MP, announced today a capital investment of £50 million towards the new development of Tate Modern. This is the largest capital commitment by Government to a cultural project since the British Library which opened in 1998. Nicholas Serota, Director Tate, said: “Today’s announcement is an important endorsement by Government of the contribution that the arts make to society as a whole and the importance of British art at an international level. It gives us a platform for the creation of an institution for the 21st century, designed to serve the next generation of artists and visitors. This commitment confirms London’s position as one of the leading international centres for the visual arts.” The Success of Tate Modern In 2000, an investment of £137 million of public and private money created Tate Modern. In just seven years, the gallery has become most popular museum of modern art in the world. It has attracted over 30 million visitors since it opened and is Britain’s second leading tourist attraction. Around 60% of visitors are under 35 years of age. The gallery has helped revive a wide area of inner London, helping to reconfigure cultural tourism along the South Bank, creating up to 4,000 new jobs. The Need to Develop Tate Modern In spite of its success, much of Tate Modern’s potential is still to be realised. One third of the building remains derelict and needs to be brought into use. The building was originally designed for 1.8 million visitors a year. With present audiences at nearly 5 million, there is serious overcrowding in the galleries, particularly at weekends, and there is an urgent need to improve and extend facilities. Different kinds of galleries are required to show art forms new to Tate, including photography, video, film and performance, as well as more galleries to show major exhibitions in their entirety. Bigger spaces are needed to meet the requirements of Tate’s growing number of large-scale works and installations. With additional space, more of Tate’s Collection can go on view and key paintings, sculptures and installations can be brought out of storage and displayed on a more permanent basis. The New Building The new development, by internationally celebrated architects, Herzog & de Meuron, will create a spectacular new building adjoining Tate Modern to the south, on the foundation of the former power station’s oil tanks. This will be Britain’s most important new building for culture since the creation of the Royal National Theatre in 1976, the Barbican in 1982, and the British Library in 1998. The new building will increase Tate Modern’s size by 60% adding approximately 21,000 square metres of new space. The development will provide more space for contemporary art and enable Tate to explore new areas of contemporary visual culture involving photography, film, video and performance, enriching its current programme. Tate Modern’s outstanding and pioneering education programme will at last have the space to meet its potential and serve a new and broader audience. New Cultural Quarter The new development of Tate Modern will create a dynamic new part of London – a creative campus stretching southwards. A new entrance on the south side will open up a north-south route or ‘street’ right through the building, creating a pedestrian way from the City across the MillenniumBridge through the Turbine Hall to Southwark and the Elephant and Castle. The new development lies at the heart of London’s cultural quarter running from the London Eye to the DesignMuseum and consisting of a group of more than 20 cultural organisations. Next Steps The designs for the new building were granted planning permission by Southwark Council in March 2007 with the Planning Committee unanimous in its support for the scheme. The project will now enter its detailed design phase and a project team is being appointed. The total costs of the project are comparable to the costs of creating the original Tate Modern: £165 million in 2006 prices, £215 million at outturn in 2012. A projected additional 1 million visitors a year will increase Tate Modern’s operating revenues significantly. The Mayor of London has given a major investment of £7 million from the LDA towards the development and help fast-track the scheme so that it might be completed in time for the Olympic Games in 2012. Fundraising from the private sector is progressing well and includes the recent"""
        matches = fastmatch.match(s1,s2,15)
        printresults(matches,s1,s2)
        self.assertEqual(len(matches),96)
    
    def test_bible_against_koran(self):
        def getFile(path,url,encoding):
            if not os.path.exists(path):
                f = open(path,'w')
                f.write(urllib.urlopen(url).read())
                f.close()
            return codecs.open(path,encoding=encoding).read()
        s1 = getFile('koran.txt','http://www.gutenberg.org/ebooks/2800.txt.utf8','utf-8').split('*END*')[-1]
        s2 = getFile('bible.txt','http://www.gutenberg.org/ebooks/10','utf-8').split('*END*')[-1]
        matches = fastmatch.match(s1,s2,20)
        printresults(matches[:60],s1,s2)
        self.assertEqual(len(matches),25257)

if __name__ == "__main__":
    unittest.main()
