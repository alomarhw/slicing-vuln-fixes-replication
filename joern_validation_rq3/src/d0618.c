ExtensionFunction::ResponseValue TabsUpdateFunction::GetResult() {
  if (!has_callback())
    return NoArguments();

  return ArgumentList(
      tabs::Get::Results::Create(*ExtensionTabUtil::CreateTabObject(
          web_contents_, ExtensionTabUtil::kScrubTab, extension())));
}
