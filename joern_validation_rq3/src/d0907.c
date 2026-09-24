static int effect_release(struct effect_s *effect)
{
 return effect_set_state(effect, EFFECT_STATE_INIT);
}
