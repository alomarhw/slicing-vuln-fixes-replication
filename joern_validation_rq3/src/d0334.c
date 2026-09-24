void HTMLMediaElement::addTextTrack(WebInbandTextTrack* webTrack) {
  InbandTextTrack* textTrack = InbandTextTrack::create(webTrack);


  textTrack->setReadinessState(TextTrack::Loaded);

  scheduleTextTrackResourceLoad();

  textTracks()->append(textTrack);
}
