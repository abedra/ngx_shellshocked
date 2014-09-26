require 'spec_helper'

describe "Integration Specs" do
  describe "Use cases" do
    it "Returns 403 when shellshocked payload is in referer" do
      http = Curl.get("http://127.0.0.1:8888") do |http|
        http.headers['Referer'] = "() { :; }; echo foo"
      end

      expect(http.response_code).to eq(403)
    end

    it "Returns 400 when shellshocked payload is in host" do
      http = Curl.get("http://127.0.0.1:8888") do |http|
        http.headers['Host'] = "() { :; }; echo foo"
      end

      expect(http.response_code).to eq(403)
    end

  end
end
