import os
from flask import Flask, render_template, request
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

app = Flask(__name__)
engine = create_engine('sqlite:////opt/ipd/ipd.db', echo=True)

class Driver(object):
    def __init__(self, name, car_make, car_model):
        self.name = name
        self.car_make = car_make
        self.car_model = car_model

    def __repr__(self):
        return f'<Driver(name={self.name}, car_make={self.car_make}, car_model={self.car_model})>'

class Race(object):
    def __init__(self, driver_id, start_lat, start_lng, finish_lat, finish_lng, top_speed):
        self.driver_id = driver_id
        self.start_lat = start_lat
        self.start_lng = start_lng
        self.finish_lat = finish_lat
        self.finish_lng = finish_lng
        self.top_speed = top_speed

    def __repr__(self):
        return f'<Race(driver_id={self.driver_id}, start_lat={self.start_lat}, start_lng={self


